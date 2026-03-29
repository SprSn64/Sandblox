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
	Camera* currCam = client.gameWorld->currCamera;

	Vector4 newPos = matrixMult(matrixMult(vec3ToVec4(inputVert.pos), currCam->transform), currCam->proj);
	newPos.w = newPos.z;

	MeshVert newVert = {
		(Vector3){-newPos.x / newPos.w, newPos.y / newPos.w, -newPos.z},
		inputVert.norm,
		inputVert.uv,
		inputVert.colour
	};

	return newVert;
}
MeshVert (*currShader)(MeshVert) = defaultVertShader;

void drawDepthPixel(Texture* target, Uint16 x, Uint16 y, float z, Uint32 colour){
	if(!target || x >= target->width || y >= target->height || (colour & 0xFF000000) >> 24 == 0) return;
	int targetPixel = x + y * target->width;
	if(z <= 0 || (depthBuffer[targetPixel] < z && doZBuffer)) return;
	if(doZBuffer)depthBuffer[targetPixel] = z;
	setPixel(target, x, y, colour);
}

void drawDepthHamLine(Texture* target, Vector3 pointA, Vector3 pointB, Uint32 colourA, Uint32 colourB){
	float scaler = max(fabs(pointB.x - pointA.x), fabs(pointB.y - pointA.y));
	if(scaler <= 0 || !target) return;

	int loopCount = ((pointB.x - pointA.x) >= (pointB.y - pointA.y) ? 
		min(pointB.x - pointA.x, target->width): 
		min(pointB.y - pointA.y, target->height));

	for(int i=0; i < min(loopCount, target->width); i++){
		float lerpVal = i/scaler;
		drawDepthPixel(target, 
			lerp(pointA.x, pointB.x, lerpVal), 
			lerp(pointA.y, pointB.y, lerpVal), 
			lerp(pointA.z, pointB.z, lerpVal),
			colourLerp(colourA, colourB, lerpVal)
		);
	}
}

void drawDepthBar(Texture* target, int y, SDL_FPoint pointA, SDL_FPoint pointB, Uint32 colourA, Uint32 colourB){
	float lerpVal = (float)1/(pointB.x - pointA.x);
	for(int i=0; i < min(pointB.x - pointA.x, target->width); i++){
		drawDepthPixel(target, pointA.x + i, y, lerp(pointA.y, pointB.y, i * lerpVal), colourLerp(colourA, colourB, i * lerpVal));
	}
}

Vector3 toScreen(Texture* target, Vector3 pos){
	return (Vector3){(pos.x + 1) * (target->width >> 1), (pos.y + 1) * (target->height >> 1), pos.z};
}

//currently only draws bottom half of the triangles with flat colouring
void drawDepthTriangle(Texture* target, MeshVert vertA, MeshVert vertB, MeshVert vertC, Texture* tex){
	(void)tex;
	if(!target) return;
	MeshVert newVerts[3] = {currShader(vertA), currShader(vertB), currShader(vertC)};

	float zoomScale = client.gameWorld->currCamera->zoom;
	Vector3 posA = toScreen(target, (Vector3){newVerts[0].pos.x * zoomScale, newVerts[0].pos.y * zoomScale, newVerts[0].pos.z});
	Vector3 posB = toScreen(target, (Vector3){newVerts[1].pos.x * zoomScale, newVerts[1].pos.y * zoomScale, newVerts[1].pos.z});
	Vector3 posC = toScreen(target, (Vector3){newVerts[2].pos.x * zoomScale, newVerts[2].pos.y * zoomScale, newVerts[2].pos.z});

	if(max(posA.z, max(posB.z, posC.z)) <= 0 || min(posA.z, min(posB.z, posC.z)) >= 1024) return;

	if((posB.x - posA.x) * (posB.y + posA.y) + (posC.x - posB.x) * (posC.y + posB.y) + (posA.x - posC.x) * (posA.y + posC.y) < 0) return;

	//Replace this stuff with triangle filling algorithm
	/*drawDepthHamLine(target, posA, posB, colourToInt(newVerts[0].colour), colourToInt(newVerts[1].colour));
	drawDepthHamLine(target, posB, posC, colourToInt(newVerts[1].colour), colourToInt(newVerts[2].colour));
	drawDepthHamLine(target, posC, posA, colourToInt(newVerts[2].colour), colourToInt(newVerts[0].colour));
	drawDepthPixel(target, 
		(posA.x + posB.x + posC.x)/3,
		(posA.y + posB.y + posC.y)/3,
		(posA.z + posB.z + posC.z)/3,
		colourToInt(newVerts[0].colour)
	);*/

	int triTop = min(posA.y, min(posB.y, posC.y));
	int triHeight = max(posA.y, max(posB.y, posC.y)) - triTop;
	Uint32 triColour = colourToInt((SDL_FColor){
		(newVerts[0].colour.r + newVerts[1].colour.r + newVerts[2].colour.r) * 0.3333,
		(newVerts[0].colour.g + newVerts[1].colour.g + newVerts[2].colour.g) * 0.3333,
		(newVerts[0].colour.b + newVerts[1].colour.b + newVerts[2].colour.b) * 0.3333,
		(newVerts[0].colour.a + newVerts[1].colour.a + newVerts[2].colour.a) * 0.3333,
	});

	for(int i=0; i<triHeight; i++){
		float triLerp[3] = {invLerp(posA.y, posB.y, triTop + i), invLerp(posB.y, posC.y, triTop + i), invLerp(posC.y, posA.y, triTop + i)};
		
		if(between(triLerp[0], 0, 1) && between(triLerp[1], 0, 1)){
			drawDepthBar(target, triTop + i, 
				(SDL_FPoint){lerp(posA.x, posB.x, triLerp[0]), lerp(posA.z, posB.z, triLerp[0])},  
				(SDL_FPoint){lerp(posB.x, posC.x, triLerp[1]), lerp(posB.z, posC.z, triLerp[1])}, 
				triColour, triColour
			);
			/*drawDepthBar(target, triTop + i,
				(SDL_FPoint){lerp(posA.x, posB.x, 1-triLerp[0]), lerp(posA.z, posB.z, 1-triLerp[0])}, 
				(SDL_FPoint){lerp(posB.x, posC.x, 1-triLerp[1]), lerp(posB.z, posC.z, 1-triLerp[1])}, 
				triColour, triColour
			);*/
		}

		if(between(triLerp[1], 0, 1) && between(triLerp[2], 0, 1)){
			drawDepthBar(target, triTop + i,
				(SDL_FPoint){lerp(posB.x, posC.x, triLerp[1]), lerp(posB.z, posC.z, triLerp[1])}, 
				(SDL_FPoint){lerp(posC.x, posA.x, triLerp[2]), lerp(posC.z, posA.z, triLerp[2])}, 
				triColour, triColour
			);
		}

		if(between(triLerp[2], 0, 1) && between(triLerp[0], 0, 1)){
			drawDepthBar(target, triTop + i,
				(SDL_FPoint){lerp(posC.x, posA.x, triLerp[2]), lerp(posC.z, posA.z, triLerp[2])}, 
				(SDL_FPoint){lerp(posA.x, posB.x, triLerp[0]), lerp(posA.z, posB.z, triLerp[0])}, 
				triColour, triColour
			);
		}
	}
}