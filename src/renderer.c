#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "renderer.h"
#include "math.h"

extern SDL_Renderer *renderer;

extern float timer;
extern ClientData client;

//extern SDL_Point windowScaleIntent;
//extern double windowScaleFactor;
extern SDL_Point windowScale;

SortTri *triListHead = NULL;
SortTri *triListLast = NULL;
Uint32 triListLength = 0;

float renderScale = 480;
Vector3 lightNormal = (Vector3){0.25, 0.42, 0.33};
SDL_FColor lightColour = {1, 1, 1, 1};
SDL_FColor lightAmbient = {0.1, 0.1, 0.15, 1};

Vector3 worldToCamera(Vector3 pos){
	Vector3 firstPos = {pos.x - client.gameWorld->currCamera->pos.x, pos.y - client.gameWorld->currCamera->pos.y, pos.z - client.gameWorld->currCamera->pos.z};
	Vector3 newPos;
		newPos.x = firstPos.x * SDL_cos(client.gameWorld->currCamera->rot.y) + firstPos.z * -SDL_sin(client.gameWorld->currCamera->rot.y); newPos.z = firstPos.x * SDL_sin(client.gameWorld->currCamera->rot.y) + firstPos.z * SDL_cos(client.gameWorld->currCamera->rot.y);
		newPos.y = firstPos.y * SDL_cos(client.gameWorld->currCamera->rot.x) + newPos.z * SDL_sin(client.gameWorld->currCamera->rot.x); newPos.z = firstPos.y * -SDL_sin(client.gameWorld->currCamera->rot.x) + newPos.z * SDL_cos(client.gameWorld->currCamera->rot.x);
		newPos.x = newPos.x * SDL_cos(client.gameWorld->currCamera->rot.z) + newPos.y * -SDL_sin(client.gameWorld->currCamera->rot.z); newPos.y = newPos.x * SDL_sin(client.gameWorld->currCamera->rot.z) + newPos.y * SDL_cos(client.gameWorld->currCamera->rot.z);
	return newPos;
}

Vector3 isoProj(Vector3 posA){
	Vector3 pos = worldToCamera(posA);
	return (Vector3){(pos.x + (pos.z/2)) * 32 + windowScale.x / 2, (-pos.y + pos.z / 2) * 32 + windowScale.y / 2, pos.z};
	//return (Vector3){(-pos.x / (pos.z - 4)) * (windowScaleFactor * 32) + windowScale.x / 2, ((pos.y - 4) / (pos.z - 4)) * (windowScaleFactor * 32) + windowScale.y / 2, pos.z};
}

Vector3 viewProj(Vector3 pos){
	return (Vector3){pos.x / pos.z * client.gameWorld->currCamera->zoom, pos.y / pos.z * client.gameWorld->currCamera->zoom, pos.z * client.gameWorld->currCamera->zoom};
	//Vector4 matrixed = matrixMult((Vector4){pos.x, pos.y, pos.z, 1}, client.gameWorld->currCamera->transform);
	//return (Vector3){matrixed.x, matrixed.y, matrixed.z};
}

Vector3 projToScreen(Vector3 pos){
	return (Vector3){-pos.x * client.gameWorld->currCamera->zoom * renderScale + windowScale.x / 2, pos.y * client.gameWorld->currCamera->zoom * renderScale + windowScale.y / 2, pos.z};
}

bool drawTriangle(Vector3 pointA, Vector3 pointB, Vector3 pointC, SDL_FColor colour){
	SDL_Vertex verts[3];
	verts[0].position = (SDL_FPoint){pointA.x, pointA.y}; verts[0].color = colour;
	verts[1].position = (SDL_FPoint){pointB.x, pointB.y}; verts[1].color = colour;
	verts[2].position = (SDL_FPoint){pointC.x, pointC.y}; verts[2].color = colour;
	float clockwiseAB = (verts[1].position.x - verts[0].position.x) * (verts[1].position.y + verts[0].position.y);
	float clockwiseBC = (verts[2].position.x - verts[1].position.x) * (verts[2].position.y + verts[1].position.y);
	float clockwiseCA = (verts[0].position.x - verts[2].position.x) * (verts[0].position.y + verts[2].position.y);

	if(clockwiseAB + clockwiseBC + clockwiseCA >= 0){// && max(pointA.z, max(pointB.z, pointC.z)) >= 0){
		SDL_RenderGeometry(renderer, NULL, verts, 3, NULL, 0);
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); SDL_RenderFillRect(renderer, &(SDL_FRect){verts[1].position.x, verts[1].position.y, 2, 2});
		return 0;
	}
	return 1;
}

bool addListTri(Vector3 pointA, Vector3 pointB, Vector3 pointC, SDL_FColor colour){
	SortTri *newItem = malloc(sizeof(SortTri)); //memory leak culprit
	if(!newItem) return 1;
	newItem->vertA = pointA; newItem->vertB = pointB; newItem->vertC = pointC; newItem->colour = colour; 
	
	if(!triListHead){
		triListHead = newItem;
		goto triListEnd;
	}
	
	triListLast->next = newItem;
	
triListEnd:
	triListLength++;
	triListLast = newItem;
	return 0;
}

bool draw3DTriangle(Vector3 pointA, Vector3 pointB, Vector3 pointC, SDL_FColor colour){
	Vector3 projLoc[3] = {projToScreen(viewProj(worldToCamera(pointA))), projToScreen(viewProj(worldToCamera(pointB))), projToScreen(viewProj(worldToCamera(pointC)))};
	if(max(projLoc[0].z, max(projLoc[1].z, projLoc[2].z)) < 0){
		drawTriangle(projLoc[0], projLoc[1], projLoc[2], colour); return 0;
		
		//memory leak incoming
		//return addListTri(projLoc[0], projLoc[1], projLoc[2], colour);
	}
	return 1;
}

/*void drawCube(Vector3 pos, Vector3 scale, SDL_FColor colour){
	//side
	draw3DTriangle((Vector3){pos.x, pos.y, pos.z}, (Vector3){pos.x, pos.y, pos.z + scale.z}, (Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}, (SDL_FColor){colour.r * 0.4, colour.g * 0.4, colour.b * 0.7, 1});
	draw3DTriangle((Vector3){pos.x, pos.y - scale.y, pos.z}, (Vector3){pos.x, pos.y, pos.z}, (Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}, (SDL_FColor){colour.r * 0.4, colour.g * 0.4, colour.b * 0.7, 1});
	
	draw3DTriangle((Vector3){pos.x + scale.x, pos.y, pos.z}, (Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}, (Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}, (SDL_FColor){colour.r * 0.4, colour.g * 0.4, colour.b * 0.7, 1});
	draw3DTriangle((Vector3){pos.x + scale.x, pos.y, pos.z}, (Vector3){pos.x + scale.x, pos.y - scale.y, pos.z}, (Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}, (SDL_FColor){colour.r * 0.4, colour.g * 0.4, colour.b * 0.7, 1});
	//front
	draw3DTriangle((Vector3){pos.x, pos.y, pos.z + scale.z}, (Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}, (Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}, (SDL_FColor){colour.r * 0.76, colour.g * 0.8, colour.b * 0.9, 1});
	draw3DTriangle((Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}, (Vector3){pos.x, pos.y, pos.z + scale.z}, (Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}, (SDL_FColor){colour.r * 0.76, colour.g * 0.8, colour.b * 0.9, 1});
	
	draw3DTriangle((Vector3){pos.x + scale.x, pos.y, pos.z}, (Vector3){pos.x, pos.y, pos.z}, (Vector3){pos.x + scale.x, pos.y - scale.y, pos.z}, (SDL_FColor){colour.r * 0.76, colour.g * 0.8, colour.b * 0.9, 1});
	draw3DTriangle((Vector3){pos.x, pos.y, pos.z}, (Vector3){pos.x, pos.y - scale.y, pos.z}, (Vector3){pos.x + scale.x, pos.y - scale.y, pos.z}, (SDL_FColor){colour.r * 0.76, colour.g * 0.8, colour.b * 0.9, 1});
	//top
	draw3DTriangle(pos, (Vector3){pos.x + scale.x, pos.y, pos.z}, (Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}, colour);
	draw3DTriangle((Vector3){pos.x, pos.y, pos.z + scale.z}, pos, (Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}, colour);
	
	draw3DTriangle((Vector3){pos.x + scale.x, pos.y - scale.y, pos.z}, (Vector3){pos.x, pos.y - scale.y, pos.z}, (Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}, (SDL_FColor){colour.r * 0.28, colour.g * 0.28, colour.b * 0.6, 1});
	draw3DTriangle((Vector3){pos.x, pos.y - scale.y, pos.z}, (Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}, (Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}, (SDL_FColor){colour.r * 0.28, colour.g * 0.28, colour.b * 0.6, 1});
}*/

void drawMesh(Mesh* mesh, mat4 transform, SDL_FColor colour){
	if(!mesh) return;
	if(!transform) return;
	
	Vector4 pointCalcs[3];
	
	for(int i=0; i < mesh->faceCount; i++){
		if (mesh->faces[i].vertA && mesh->faces[i].vertB && mesh->faces[i].vertC) {
			pointCalcs[0] = matrixMult((Vector4){mesh->faces[i].vertA->pos.x, mesh->faces[i].vertA->pos.y, mesh->faces[i].vertA->pos.z, 1}, transform);
			pointCalcs[1] = matrixMult((Vector4){mesh->faces[i].vertB->pos.x, mesh->faces[i].vertB->pos.y, mesh->faces[i].vertB->pos.z, 1}, transform);
			pointCalcs[2] = matrixMult((Vector4){mesh->faces[i].vertC->pos.x, mesh->faces[i].vertC->pos.y, mesh->faces[i].vertC->pos.z, 1}, transform);
			
			SDL_FColor faceColour = {
				(mesh->faces[i].vertA->colour.r + mesh->faces[i].vertB->colour.r + mesh->faces[i].vertC->colour.r) / 3,
				(mesh->faces[i].vertA->colour.g + mesh->faces[i].vertB->colour.g + mesh->faces[i].vertC->colour.g) / 3,
				(mesh->faces[i].vertA->colour.b + mesh->faces[i].vertB->colour.b + mesh->faces[i].vertC->colour.b) / 3,
				1
			};
			Vector3 faceNormal = {
				(mesh->faces[i].vertA->norm.x + mesh->faces[i].vertB->norm.x + mesh->faces[i].vertC->norm.x) / 3,
				(mesh->faces[i].vertA->norm.y + mesh->faces[i].vertB->norm.y + mesh->faces[i].vertC->norm.y) / 3,
				(mesh->faces[i].vertA->norm.z + mesh->faces[i].vertB->norm.z + mesh->faces[i].vertC->norm.z) / 3,
			};
			SDL_FPoint faceUV = {
				(mesh->faces[i].vertA->uv.x + mesh->faces[i].vertA->uv.x) / 2,
				(mesh->faces[i].vertA->uv.y + mesh->faces[i].vertA->uv.y) / 2,
			};
			float faceDot = max(dotProd3(faceNormal, lightNormal), 0);
			Vector3 cameraNorm = rotToNorm3(client.gameWorld->currCamera->rot);
			Vector3 reflectSource = normalize3(reflect((Vector3){-lightNormal.x, -lightNormal.y, -lightNormal.z}, faceNormal));
			float specular = pow(max(dotProd3(cameraNorm, reflectSource), 0), 32);
			
			draw3DTriangle((Vector3){pointCalcs[0].x, pointCalcs[0].y, pointCalcs[0].z}, (Vector3){pointCalcs[1].x, pointCalcs[1].y, pointCalcs[1].z}, (Vector3){pointCalcs[2].x, pointCalcs[2].y, pointCalcs[2].z}, (SDL_FColor){
				(colour.r * lightAmbient.r) + (colour.r * lightColour.r * faceDot) + (specular * lightColour.r),
				(colour.g * lightAmbient.g) + (colour.g * lightColour.g * faceDot) + (specular * lightColour.g),
				(colour.b * lightAmbient.b) + (colour.b * lightColour.b * faceDot) + (specular * lightColour.b),
				colour.a
			});
			
			/*(SDL_FColor){
				(specular + colour.r) * lightColour.r * (faceDot + lightAmbient.r - lightAmbient.r * faceDot), 
				(specular + colour.g) * lightColour.g * (faceDot + lightAmbient.g - lightAmbient.g * faceDot), 
				(specular + colour.b) * lightColour.b * (faceDot + lightAmbient.b - lightAmbient.b * faceDot), colour.a
			}*/
			//draw3DTriangle((Vector3){pointCalcs[0].x, pointCalcs[0].y, pointCalcs[0].z}, (Vector3){pointCalcs[1].x, pointCalcs[1].y, pointCalcs[1].z}, (Vector3){pointCalcs[2].x, pointCalcs[2].y, pointCalcs[2].z}, (SDL_FColor){faceUV.x, faceUV.y, 0, 1});
		}
	}
}

SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode){
	SDL_Texture *texture = IMG_LoadTexture(renderer, path);
	if(texture == NULL){
		printf("Issue with loading texture %s!\n", path);
		return NULL;
	}
	SDL_SetTextureScaleMode(texture, scaleMode);
	return texture;
}

SDL_FColor ConvertSDLColour(CharColour colour){
	return (SDL_FColor){(float)colour.r / 255, (float)colour.g / 255, (float)colour.b / 255, (float)colour.a / 255};
}

CharColour ConvertColour(CharColour colour, Uint32 mode){
	return colour;
}


//fix soon
void drawBillboard(SDL_Texture *texture, SDL_FRect rect, Vector3 pos, SDL_FPoint offset, SDL_FPoint scale){
	Vector3 projLoc[3] = {projToScreen(viewProj(worldToCamera(pos))), projToScreen(viewProj(worldToCamera((Vector3){pos.x--, pos.y, pos.z}))), projToScreen(viewProj(worldToCamera((Vector3){pos.x++, pos.y, pos.z})))};
	if(projLoc[0].z < 0){
		double sizeMult = fabs(-(projLoc[2].x - projLoc[1].x) / scale.x);
		SDL_FRect sprPos = {projLoc[0].x - offset.x * sizeMult, projLoc[0].y - offset.y * sizeMult, scale.x * 3 * sizeMult, scale.y * 3 * sizeMult};
		//SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); SDL_RenderFillRect(renderer, &sprPos);
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); SDL_RenderFillRect(renderer, &(SDL_FRect){projLoc[0].x - 2, projLoc[0].y - 2, 4, 4});
		//printf("%f, %f, %f, %f, %f\n", sizeMult, sprPos.x, sprPos.y, sprPos.w, sprPos.h);
		SDL_RenderTexture(renderer, texture, &rect, &sprPos);
	}
}

void drawText(SDL_Renderer *renderer, SDL_Texture *texture, char* text, char charOff, short posX, short posY, short width, short height, short kern){
	for(int i=0; i<=strlen(text); i++){
		char charVal = (unsigned)text[i] - charOff;
		int xOff = (charVal % 16) * width;
		int yOff = floor((float)charVal / 16) * height;
		SDL_FRect sprRect = {xOff, yOff, width, height};
		SDL_FRect sprPos = {posX + kern * i, posY, width, height};
		SDL_RenderTexture(renderer, texture, &sprRect, &sprPos);
	}
}

//add stuff for adding triangles to list from mesh, sorting list based off z value then rendering to screen

int SDLCALL compare(const void *userdata, const void *a, const void *b){
	const SortTri *A = (const SortTri *)a;
	const SortTri *B = (const SortTri *)b;
	
	float avgZA = (A->vertA.z + A->vertB.z + A->vertC.z) / 3;
	float avgZB = (B->vertA.z + B->vertB.z + B->vertC.z) / 3;

	if(avgZA == avgZB) return 0;
	return 1 - (avgZA < avgZB) * 2;
}

void sortTriList(){
	//quicksort or whatever but with link list instead of regular array
	//SDL_qsort(values, SDL_arraysize(values), sizeof(values[0]), compare);
	
	//bubble sort
	SortTri *triA = triListHead; 
	SortTri *triB = triListHead->next;
	for(int i=0; i < triListLength; i++){
		float avgZA = (triA->vertA.z + triA->vertB.z + triA->vertC.z) / 3;
		float avgZB = (triB->vertA.z + triB->vertB.z + triB->vertC.z) / 3;
		if(avgZA < avgZB){
			//fuck
		}
	}
}

void renderTriList(){
	sortTriList();
	
	SortTri *loopItem = triListHead;
	for(;;){
		drawTriangle(loopItem->vertA, loopItem->vertB, loopItem->vertC, loopItem->colour);
		
		if(!loopItem->next) break;
		loopItem = loopItem->next;
	}
	
	loopItem = triListHead;
	bool stopLoop = false;
	while(!stopLoop){
		SortTri *clearItem = loopItem;
		if(!loopItem->next) stopLoop = true;
		loopItem = loopItem->next;
		free(clearItem);
	}
	
	triListLength = 0;
	triListHead, triListLast = NULL;
}