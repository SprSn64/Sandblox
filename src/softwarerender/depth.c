#include "depth.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <structs.h>

#include "main.h"
#include "../math.h"
#include "../renderer.h"

extern float* depthBuffer;
bool doZBuffer = true;

extern ClientData client;
MeshVert defaultVertShader(MeshVert inputVert){
	MeshVert newVert = {
		vec4ToVec3(matrixMult(vec3ToVec4(inputVert.pos), client.gameWorld->currCamera->transform)),
		inputVert.norm,
		inputVert.uv,
		inputVert.colour,
	};

	return newVert;
}

MeshVert (*currShader)(MeshVert) = defaultVertShader;

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

void drawDepthTriangle(Texture* target, MeshVert vertA, MeshVert vertB, MeshVert vertC, Uint32 colour){//replace Vector3s with MeshVerts
	if(!target) return;
	MeshVert newVerts[3] = {currShader(vertA), currShader(vertB), currShader(vertC)};

	float zoomScale = (1 / client.gameWorld->currCamera->focusDist) * client.gameWorld->currCamera->zoom;
	Vector3 posA = toScreen(target, (Vector3){newVerts[0].pos.x * zoomScale, -newVerts[0].pos.y * zoomScale, -newVerts[0].pos.z});
	Vector3 posB = toScreen(target, (Vector3){newVerts[1].pos.x * zoomScale, -newVerts[1].pos.y * zoomScale, -newVerts[1].pos.z});
	Vector3 posC = toScreen(target, (Vector3){newVerts[2].pos.x * zoomScale, -newVerts[2].pos.y * zoomScale, -newVerts[2].pos.z});

	if((posB.x - posA.x) * (posB.y + posA.y) + (posC.x - posB.x) * (posC.y + posB.y) + (posA.x - posC.x) * (posA.y + posC.y) < 0) return;

	//Replace this stuff with triangle filling algorithm
	drawDepthHamLine(target, posA, posB, colour);
	drawDepthHamLine(target, posB, posC, colour);
	drawDepthHamLine(target, posC, posA, colour);
	drawDepthPixel(target, 
		(posA.x + posB.x + posC.x)/3,
		(posA.y + posB.y + posC.y)/3,
		(posA.z + posB.z + posC.z)/3,
		colour
	);
}