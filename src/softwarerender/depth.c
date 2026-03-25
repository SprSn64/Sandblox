#include "depth.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <structs.h>

#include "main.h"
#include "../math.h"

extern float* depthBuffer;
bool doZBuffer = true;

void drawDepthPixel(Texture* target, Uint16 x, Uint16 y, float z, Uint32 colour){
	if(!target || x >= target->width || y >= target->height || (colour & 0xFF000000) >> 24 == 0) return;
	int targetPixel = x + y * target->width;
	if(z <= 0.1 || (depthBuffer[targetPixel] < z && doZBuffer)) return;
	if(doZBuffer)depthBuffer[targetPixel] = z;
	setPixel(target, x, y, colour);
}

void drawDepthHamLine(Texture* target, Vector3 pointA, Vector3 pointB, Uint32 colour){
	float scaler = max(fabs(pointB.x - pointA.x), fabs(pointB.y - pointA.y));
	if(scaler <= 0 || !target) return;

	int loopCount = ((pointB.x - pointA.x) >= (pointB.y - pointA.y) ? 
		min(pointB.x - pointA.x, target->width): 
		min(pointB.y - pointA.y, target->height));

	for(int i=0; i < loopCount; i++){
		float lerpVal = i/scaler;
		drawDepthPixel(target, 
			lerp(pointA.x, pointB.x, lerpVal), 
			lerp(pointA.y, pointB.y, lerpVal), 
			lerp(pointA.z, pointB.z, lerpVal),
			colour
		);
	}
}

Vector3 toScreen(Texture* target, Vector3 pos){
	return (Vector3){(pos.x + 1) * (target->width >> 1), (pos.y + 1) * (target->height >> 1), pos.z};
}

extern Camera currentCamera;
void drawDepthTriangle(Texture* target, Vector3 pointA, Vector3 pointB, Vector3 pointC, Uint32 colour){
	if(!target) return;
	float zoomScale = (1 / currentCamera.focusDist);
	Vector3 posA = toScreen(target, (Vector3){pointA.x * zoomScale, -pointA.y * zoomScale, -pointA.z});
	Vector3 posB = toScreen(target, (Vector3){pointB.x * zoomScale, -pointB.y * zoomScale, -pointB.z});
	Vector3 posC = toScreen(target, (Vector3){pointC.x * zoomScale, -pointC.y * zoomScale, -pointC.z});

	float clockwiseAB = (posB.x - posA.x) * (posB.y + posA.y);
	float clockwiseBC = (posC.x - posB.x) * (posC.y + posB.y);
	float clockwiseCA = (posA.x - posC.x) * (posA.y + posC.y);

	if(clockwiseAB + clockwiseBC + clockwiseCA < 0) return;

	drawDepthHamLine(target, posA, posB, colour);
	drawDepthHamLine(target, posB, posC, colour);
	drawDepthHamLine(target, posC, posA, colour);
}