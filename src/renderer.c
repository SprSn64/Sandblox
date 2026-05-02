#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "renderer.h"
#include "math.h"
#include "opengl.h"

#include "softwarerender/main.h"
#include "softwarerender/depth.h"

extern SDL_Renderer *renderer;
extern bool glEnabled;

extern float timer;
extern ClientData client;

//extern SDL_Point windowScaleIntent;
//extern double windowScaleFactor;
extern SDL_Point windowScale;

Vector3 lightNormal = (Vector3){0.25, 0.42, 0.33};
SDL_FColor lightColour = {1, 1, 1, 1};
SDL_FColor lightAmbient = {0.25, 0.25, 0.3, 1};

//bool matrixOrSlopProject = false;
extern float* defaultMatrix;

SDL_FColor clampColour(SDL_FColor colour){
	return (SDL_FColor){min(max(colour.r, 0), 1), min(max(colour.g, 0), 1), min(max(colour.b, 0), 1), min(max(colour.a, 0), 1)};
}

SDL_FColor ConvertSDLColour(CharColour colour){
	return (SDL_FColor){(float)colour.r / 255, (float)colour.g / 255, (float)colour.b / 255, (float)colour.a / 255};
}

CharColour ConvertColour(CharColour colour, Uint32 mode){
	(void)mode;
	return colour;
}

extern Mesh* planePrim;
//fix soon
void drawBillboard(TextureRef *texture, SDL_FRect rect, Vector3 pos, SDL_FPoint offset, SDL_FPoint scale){
	(void)rect; (void)offset;
	
	Vector3 planeRot = (Vector3){
		client.gameWorld->currCamera->rot.x + HALFPI,
		client.gameWorld->currCamera->rot.y,
		0,
	};
	float* transform = genMatrix(pos, (Vector3){scale.x, 1, scale.y}, planeRot);
	drawMeshOpenGL(planePrim, transform, (SDL_FColor){1, 1, 1, 1}, texture);
	free(transform);
}

void drawText(SDL_Renderer *renderLoc, Font *textFont, char* text, short posX, short posY, float scale, SDL_FColor colour){
	SDL_SetTextureColorMod(textFont->texture->image, (Uint8)(colour.r * 255), (Uint8)(colour.g * 255), (Uint8)(colour.b * 255));
	if(!renderLoc){
		//printf("cant draw text '%s'\n", text);
		return;
	}
	for(size_t i=0; i<=strlen(text); i++){
		char charVal = text[i] - textFont->startChar;
		int xOff = (charVal % textFont->columns) * textFont->glyphSize.x;
		int yOff = floor((float)charVal / textFont->columns) * textFont->glyphSize.y;
		SDL_FRect sprRect = {xOff, yOff, textFont->glyphSize.x, textFont->glyphSize.y};
		SDL_FRect sprPos = {posX + textFont->kerning.x * i * scale, posY + textFont->kerning.y * i * scale, textFont->renderSize.x * scale, textFont->renderSize.y * scale};
		SDL_RenderTexture(renderLoc, textFont->texture->image, &sprRect, &sprPos);
	}
}

Mesh* genTorusMesh(float outerRad, float innerRad, Uint16 ringRes, Uint16 ringCount){
	if(ringRes < 3 || ringCount < 3) return NULL;
	Uint32 vertCount = ringRes * ringCount;
	Uint32 faceCount = vertCount * 2;
	
	MeshVert* newVerts = malloc(sizeof(MeshVert) * vertCount);
	MeshFace* newFaces = malloc(sizeof(MeshFace) * faceCount);
	
	float angleRes = PI * 2 / ringRes;
	float angleRing = PI * 2 / ringCount;
	for(Uint32 i=0; i<vertCount; i++){
		float newAngle = angleRing * floor(i / ringRes);
		newVerts[i].pos = (Vector3){
			(outerRad + SDL_cos(angleRes * (i % ringRes)) * innerRad) * SDL_cos(newAngle), 
			SDL_sin(angleRes * (i % ringRes)) * innerRad, 
			(outerRad + SDL_cos(angleRes * (i % ringRes)) * innerRad) * SDL_sin(newAngle),
		};
		newVerts[i].norm = (Vector3){
			SDL_cos(angleRes * (i % ringRes)) * SDL_cos(newAngle),
			SDL_sin(angleRes * (i % ringRes)),
			SDL_cos(angleRes * (i % ringRes)) * SDL_sin(newAngle),
		};
		newVerts[i].uv = (SDL_FPoint){
			newAngle / (2 * PI),
			angleRes * (i % ringRes) / (2 * PI),
		};
	}
	
	for(Uint32 i=0; i<faceCount/2; i++){
		Uint32 newI = i * 2;
		//MeshVert *quadVerts[4] = {&newVerts[i], &newVerts[(i+1) % vertCount], &newVerts[(i + ringRes) % vertCount], &newVerts[(i+1 + ringRes) % vertCount]};
		
		newFaces[newI] = (MeshFace){i, (i+1) % vertCount, (i + ringRes) % vertCount};
		newFaces[(newI + 1)] = (MeshFace){(i + ringRes) % vertCount, (i+1) % vertCount, (i+1 + ringRes) % vertCount};
	}
	
	Mesh* newMesh = malloc(sizeof(Mesh)); newMesh->persistent = false;
	newMesh->vertCount = vertCount; newMesh->verts = newVerts; newMesh->faceCount = faceCount; newMesh->faces = newFaces; 
	newMesh->meshType = MESHTYPE_TORUS;
	return newMesh;
}

Mesh* genCylinderMesh(float btmRad, float topRad, float length, int res){
	if(fabs(btmRad) + fabs(topRad) == 0 || length == 0 || res < 3) return NULL;
	Uint32 vertCount = res * 2;
	Uint32 faceCount = 4 * res - 4;
	
	MeshVert* newVerts = malloc(sizeof(MeshVert) * vertCount);
	MeshFace* newFaces = malloc(sizeof(MeshFace) * faceCount);
	
	float angleRes = PI * 2 / res;
	for(Uint32 i=0; i<vertCount; i++){
		float sideRad = lerp(btmRad, topRad, floor(i / res));
		newVerts[i].pos = (Vector3){
			SDL_cos(angleRes * (i % res)) * sideRad,
			(1 - 2 * floor(i / res)) * length / 2,
			SDL_sin(angleRes * (i % res)) * sideRad,
		};
		newVerts[i].norm = normalize3((Vector3){
			SDL_cos(angleRes * (i % res)),
			(1 - floor(i / res) * 2) / 2,
			SDL_sin(angleRes * (i % res)),
		});
	}
	
	for(int i=0; i<res; i++){
		int newI = i * 2;
		//MeshVert *quadVerts[4] = {&newVerts[i], &newVerts[(i+1) % res], &newVerts[(i + res)], &newVerts[(i+1)%res + res]};
		
		newFaces[newI] = (MeshFace){i, (i+1) % res, i + res};
		newFaces[(newI + 1)] = (MeshFace){i + res, (i+1) % res, (i+1)%res + res};
	}
	
	for(int i=0; i<res-2; i++){
		int newI = 2 * res + i;
		newFaces[newI] = (MeshFace){i, 0, (i + 1) % res};
		newFaces[newI + res - 2] = (MeshFace){res, i + res, res + (i+1) % res};
	}
	
	Mesh* newMesh = malloc(sizeof(Mesh)); newMesh->persistent = false;
	newMesh->vertCount = vertCount; newMesh->verts = newVerts; newMesh->faceCount = faceCount; newMesh->faces = newFaces; 
	newMesh->meshType = MESHTYPE_CYLINDER;
	return newMesh;
}

Mesh* genPlaneMesh(float xScale, float yScale, Uint16 xRes, Uint16 yRes){
	if(fabs(xScale) + fabs(yScale) == 0 || xRes + yRes == 0) return NULL;
	
	Uint32 vertCount = (xRes + 1) * (yRes + 1);
	Uint32 faceCount = xRes * yRes * 2;
	
	MeshVert* newVerts = malloc(sizeof(MeshVert) * vertCount);
	MeshFace* newFaces = malloc(sizeof(MeshFace) * faceCount);
	
	for(Uint32 i=0; i<vertCount; i++){
		newVerts[i].pos = (Vector3){
			floor(i / (xRes + 1)) * (xScale / xRes), 
			0, 
			(i % (xRes + 1)) * (yScale / yRes),
		};
		newVerts[i].norm = (Vector3){0, 1, 0};
		newVerts[i].uv = (SDL_FPoint){floor(i / (xRes + 1)) / xRes, (i % (xRes + 1)) / yRes};
	}
	for(Uint32 i=0; i<faceCount/2; i++){
		Uint32 newI = i * 2;
		
		newFaces[newI] = (MeshFace){i, i+1, i + xRes + 1};
		newFaces[(newI + 1)] = (MeshFace){i + xRes + 1, i+1, i + 2 + xRes};
	}
	
	Mesh* newMesh = malloc(sizeof(Mesh)); newMesh->persistent = false;
	newMesh->vertCount = vertCount; newMesh->verts = newVerts; newMesh->faceCount = faceCount; newMesh->faces = newFaces; 
	newMesh->meshType = MESHTYPE_PLANE;
	return newMesh;
}

void setDrawColour(SDL_Renderer *render, CharColour colour){
	SDL_SetRenderDrawColor(render, colour.r, colour.g, colour.b, colour.a); 
}