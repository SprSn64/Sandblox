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
	if(scaler <= 1 || scaler >= target->width * target->height) return;

	for(int i=0; i < ((pointB.x - pointA.x) >= (pointB.y - pointA.y) ? pointB.x - pointA.x : pointB.y - pointA.y); i++){
		float lerpVal = i/scaler;
		drawDepthPixel(target, 
			lerp(pointA.x, pointB.x, lerpVal), 
			lerp(pointA.y, pointB.y, lerpVal), 
			lerp(pointA.z, pointB.z, lerpVal),
			colour
		);
	}
}

extern Camera currentCamera;
void drawDepthTriangle(Texture* target, Vector3 pointA, Vector3 pointB, Vector3 pointC, Uint32 colour){
	float zoomScale = (256 / currentCamera.focusDist);
	Vector3 posA = {pointA.x * zoomScale + 160, -pointA.y * zoomScale + 120, -pointA.z};
	Vector3 posB = {pointB.x * zoomScale + 160, -pointB.y * zoomScale + 120, -pointB.z};
	Vector3 posC = {pointC.x * zoomScale + 160, -pointC.y * zoomScale + 120, -pointC.z};

	float clockwiseAB = (posB.x - posA.x) * (posB.y + posA.y);
	float clockwiseBC = (posC.x - posB.x) * (posC.y + posB.y);
	float clockwiseCA = (posA.x - posC.x) * (posA.y + posC.y);

	if(clockwiseAB + clockwiseBC + clockwiseCA < 0) return;

	drawDepthHamLine(target, posA, posB, colour);
	drawDepthHamLine(target, posB, posC, colour);
	drawDepthHamLine(target, posC, posA, colour);
}