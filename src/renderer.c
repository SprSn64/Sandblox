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

float renderScale = 480;
Vector3 lightNormal = (Vector3){0.25, 0.42, 0.33};
SDL_FColor lightColour = {1, 1, 1, 1};
SDL_FColor lightAmbient = {0.25, 0.25, 0.3, 1};

Vector3 worldToCamera(Vector3 pos){
	Vector4 firstPos = {pos.x - client.gameWorld->currCamera->pos.x, pos.y - client.gameWorld->currCamera->pos.y, pos.z - client.gameWorld->currCamera->pos.z, 1};
	Vector3 newPos;
		newPos.x = firstPos.x * SDL_cos(client.gameWorld->currCamera->rot.y) + firstPos.z * -SDL_sin(client.gameWorld->currCamera->rot.y); newPos.z = firstPos.x * SDL_sin(client.gameWorld->currCamera->rot.y) + firstPos.z * SDL_cos(client.gameWorld->currCamera->rot.y);
		newPos.y = firstPos.y * SDL_cos(client.gameWorld->currCamera->rot.x) + newPos.z * SDL_sin(client.gameWorld->currCamera->rot.x); newPos.z = firstPos.y * -SDL_sin(client.gameWorld->currCamera->rot.x) + newPos.z * SDL_cos(client.gameWorld->currCamera->rot.x);
		float tempX = newPos.x * SDL_cos(client.gameWorld->currCamera->rot.z) + newPos.y * -SDL_sin(client.gameWorld->currCamera->rot.z); newPos.y = newPos.x * SDL_sin(client.gameWorld->currCamera->rot.z) + newPos.y * SDL_cos(client.gameWorld->currCamera->rot.z); newPos.x = tempX;
	//Vector4 newPos = matrixMult((Vector4){pos.x, pos.y, pos.z, 1}, client.gameWorld->currCamera->transform);
	return (Vector3){newPos.x, newPos.y, newPos.z};
	//return newPos;
}

Vector3 viewProj(Vector3 pos){
	float absZ = fabs(pos.z);
	if(absZ < 0.001f) absZ = 0.001f;
	float safeZ = (pos.z < 0) ? -absZ : absZ;
	
	return (Vector3){pos.x / safeZ * client.gameWorld->currCamera->zoom, pos.y / safeZ * client.gameWorld->currCamera->zoom, pos.z * client.gameWorld->currCamera->zoom};
	//Vector4 matrixed = matrixMult((Vector4){pos.x, pos.y, pos.z, 1}, client.gameWorld->currCamera->transform);
	//return (Vector3){matrixed.x, matrixed.y, matrixed.z};
}

Vector3 projToScreen(Vector3 pos){
	return (Vector3){-pos.x * client.gameWorld->currCamera->zoom * renderScale + windowScale.x / 2, pos.y * client.gameWorld->currCamera->zoom * renderScale + windowScale.y / 2, pos.z};
}

extern SDL_Texture* cowTex;

bool drawTriangle(Vector3 pointA, Vector3 pointB, Vector3 pointC, SDL_FColor colour, SDL_Texture* image){
	SDL_Vertex verts[3];
	verts[0].position = (SDL_FPoint){pointA.x, pointA.y}; verts[0].color = colour;
	verts[1].position = (SDL_FPoint){pointB.x, pointB.y}; verts[1].color = colour;
	verts[2].position = (SDL_FPoint){pointC.x, pointC.y}; verts[2].color = colour;
	
	verts[0].tex_coord = (SDL_FPoint){0, 0};
	verts[1].tex_coord = (SDL_FPoint){0, 1};
	verts[2].tex_coord = (SDL_FPoint){1, 0};
	
	float clockwiseAB = (verts[1].position.x - verts[0].position.x) * (verts[1].position.y + verts[0].position.y);
	float clockwiseBC = (verts[2].position.x - verts[1].position.x) * (verts[2].position.y + verts[1].position.y);
	float clockwiseCA = (verts[0].position.x - verts[2].position.x) * (verts[0].position.y + verts[2].position.y);

	if(clockwiseAB + clockwiseBC + clockwiseCA >= 0){// && max(pointA.z, max(pointB.z, pointC.z)) >= 0){
		SDL_RenderGeometry(renderer, cowTex, verts, 3, NULL, 0);
		/*SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); 
		SDL_RenderFillRect(renderer, &(SDL_FRect){floor(verts[1].position.x), floor(verts[1].position.y), 2, 2});
		SDL_RenderFillRect(renderer, &(SDL_FRect){floor(verts[2].position.x), floor(verts[2].position.y), 2, 2});
		SDL_RenderFillRect(renderer, &(SDL_FRect){floor(verts[0].position.x), floor(verts[0].position.y), 2, 2});*/
		return 0;
	}
	return 1;
}

Vector3 clipToNearPlane(Vector3 front, Vector3 back, float nearZ){
	// front.z < nearZ (in front), back.z >= nearZ (behind or at near plane)
	float t = (nearZ - front.z) / (back.z - front.z); //collect my invLerp(a, b, v) function
	return (Vector3){
		front.x + t * (back.x - front.x), //collect my lerp(a, b, t) function
		front.y + t * (back.y - front.y),
		nearZ
	};
}

bool draw3DTriangle(Vector3 pointA, Vector3 pointB, Vector3 pointC, SDL_FColor colour, SDL_Texture* image){
	Vector3 camA = worldToCamera(pointA);
	Vector3 camB = worldToCamera(pointB);
	Vector3 camC = worldToCamera(pointC);
	
	const float NEAR_PLANE = -0.1f;  // near plane in camera space (negative = in front)
	
	bool aFront = camA.z < NEAR_PLANE;
	bool bFront = camB.z < NEAR_PLANE;
	bool cFront = camC.z < NEAR_PLANE;
	int inFront = aFront + bFront + cFront;
	
	if(inFront == 0){
		return 1;  // all behind camera, don't draw
	}
	
	if(inFront == 3){
		// all in front, draw normally
		Vector3 projLoc[3] = {projToScreen(viewProj(camA)), projToScreen(viewProj(camB)), projToScreen(viewProj(camC))};
		if(max(projLoc[0].z, max(projLoc[1].z, projLoc[2].z)) < 0){
			drawTriangle(projLoc[0], projLoc[1], projLoc[2], colour, image);
		}
		return 0;
	}
	
	/*if(inFront == 2){
		// 2 verts in front, draw as a quad
		
		// i have no idea how to make it into a quad with this current setup
		// we should probably just get opengl working at this point	
		
		return 0;
	}*/
	
	// mixed case: clip vertices behind the near plane to the near plane
	Vector3 clippedA = aFront ? camA : clipToNearPlane(bFront ? camB : camC, camA, NEAR_PLANE);
	Vector3 clippedB = bFront ? camB : clipToNearPlane(aFront ? camA : camC, camB, NEAR_PLANE);
	Vector3 clippedC = cFront ? camC : clipToNearPlane(aFront ? camA : camB, camC, NEAR_PLANE);
	
	Vector3 projLoc[3] = {projToScreen(viewProj(clippedA)), projToScreen(viewProj(clippedB)), projToScreen(viewProj(clippedC))};
	if(max(projLoc[0].z, max(projLoc[1].z, projLoc[2].z)) < 0){
		drawTriangle(projLoc[0], projLoc[1], projLoc[2], colour, image);
	}
	return 0;
}

void drawMesh(Mesh* mesh, mat4 transform, SDL_FColor colour, SDL_Texture* texture, bool shaded){
	if(!mesh || !transform) return;
	
	Vector4 pointCalcs[3];
	mat4 rotMatrix; memcpy(rotMatrix, transform, sizeof(mat4));
	rotMatrix[3] = 0; rotMatrix[7] = 0; rotMatrix[11] = 0;  
	Vector3 matrixScale = extractScale(transform);
	rotMatrix[0] = rotMatrix[0] / matrixScale.x; rotMatrix[4] = rotMatrix[4] / matrixScale.x; rotMatrix[8] = rotMatrix[8] / matrixScale.x; 
	rotMatrix[1] = rotMatrix[1] / matrixScale.y; rotMatrix[5] = rotMatrix[5] / matrixScale.y; rotMatrix[9] = rotMatrix[9] / matrixScale.y; 
	rotMatrix[2] = rotMatrix[2] / matrixScale.z; rotMatrix[6] = rotMatrix[6] / matrixScale.z; rotMatrix[10] = rotMatrix[10] / matrixScale.z; 
	
	for(Uint32 i=0; i < mesh->faceCount; i++){
		if (!(mesh->faces[i].vertA && mesh->faces[i].vertB && mesh->faces[i].vertC)) continue;
		
		pointCalcs[0] = matrixMult((Vector4){mesh->faces[i].vertA->pos.x, mesh->faces[i].vertA->pos.y, mesh->faces[i].vertA->pos.z, 1}, transform);
		pointCalcs[1] = matrixMult((Vector4){mesh->faces[i].vertB->pos.x, mesh->faces[i].vertB->pos.y, mesh->faces[i].vertB->pos.z, 1}, transform);
		pointCalcs[2] = matrixMult((Vector4){mesh->faces[i].vertC->pos.x, mesh->faces[i].vertC->pos.y, mesh->faces[i].vertC->pos.z, 1}, transform);
		
		SDL_FColor shadedColour = colour;
		if(!shaded) goto unshadedSkip;
		
		Vector4 multFaceNormal = matrixMult((Vector4){
			(mesh->faces[i].vertA->norm.x + mesh->faces[i].vertB->norm.x + mesh->faces[i].vertC->norm.x) / 3,
			(mesh->faces[i].vertA->norm.y + mesh->faces[i].vertB->norm.y + mesh->faces[i].vertC->norm.y) / 3,
			(mesh->faces[i].vertA->norm.z + mesh->faces[i].vertB->norm.z + mesh->faces[i].vertC->norm.z) / 3,
			1
		}, rotMatrix);
		Vector3 faceNormal = normalize3((Vector3){multFaceNormal.x / matrixScale.x, multFaceNormal.y / matrixScale.y, multFaceNormal.z / matrixScale.z});
		float faceDot = max(dotProd3(faceNormal, lightNormal), 0);
		Vector3 cameraNorm = rotToNorm3(client.gameWorld->currCamera->rot);
		Vector3 reflectSource = normalize3(reflect((Vector3){-lightNormal.x, -lightNormal.y, -lightNormal.z}, faceNormal));
		float specular = pow(max(dotProd3(cameraNorm, reflectSource), 0), 32);
		shadedColour = (SDL_FColor){
			(colour.r * lightAmbient.r) + (colour.r * lightColour.r * faceDot) + (specular * lightColour.r),
			(colour.g * lightAmbient.g) + (colour.g * lightColour.g * faceDot) + (specular * lightColour.g),
			(colour.b * lightAmbient.b) + (colour.b * lightColour.b * faceDot) + (specular * lightColour.b),
			colour.a
		};
		
		unshadedSkip:
		
		draw3DTriangle((Vector3){pointCalcs[0].x, pointCalcs[0].y, pointCalcs[0].z}, (Vector3){pointCalcs[1].x, pointCalcs[1].y, pointCalcs[1].z}, (Vector3){pointCalcs[2].x, pointCalcs[2].y, pointCalcs[2].z}, shadedColour, texture);
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
	(void)mode;
	return colour;
}

extern Mesh* planePrim;
//fix soon
void drawBillboard(SDL_Texture *texture, SDL_FRect rect, Vector3 pos, SDL_FPoint offset, SDL_FPoint scale){
	Vector3 planeRot = (Vector3){
		client.gameWorld->currCamera->rot.x + HALFPI,
		client.gameWorld->currCamera->rot.y,
		0,
	};
	float* transform = genMatrix(pos, (Vector3){scale.x, 1, scale.y}, planeRot);
	drawMesh(planePrim, transform, (SDL_FColor){1, 0, 1, 1}, NULL, true);
	free(transform);
	
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
	for(size_t i=0; i<=strlen(text); i++){
		char charVal = (unsigned)text[i] - charOff;
		int xOff = (charVal % 16) * width;
		int yOff = floor((float)charVal / 16) * height;
		SDL_FRect sprRect = {xOff, yOff, width, height};
		SDL_FRect sprPos = {posX + kern * i, posY, width, height};
		SDL_RenderTexture(renderer, texture, &sprRect, &sprPos);
	}
}

Mesh* genTorusMesh(float outerRad, float innerRad, int ringRes, int ringCount){
	if(ringRes < 3 || ringCount < 3) return NULL;
	Uint32 vertCount = ringRes * ringCount;
	Uint32 faceCount = vertCount * 2;
	
	MeshVert* newVerts = malloc(sizeof(MeshVert) * vertCount);
	MeshFace* newFaces = malloc(sizeof(MeshFace) * faceCount);
	
	float angleRes = PI * 2 / ringRes;
	float angleRing = PI * 2 / ringCount;
	for(int i=0; i<vertCount; i++){
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
	}
	
	for(Uint32 i=0; i<faceCount/2; i++){
		Uint32 newI = i * 2;
		MeshVert *quadVerts[4] = {&newVerts[i], &newVerts[(i+1) % vertCount], &newVerts[(i + ringRes) % vertCount], &newVerts[(i+1 + ringRes) % vertCount]};
		
		newFaces[newI] = (MeshFace){quadVerts[0], quadVerts[1], quadVerts[2]};
		newFaces[(newI + 1)] = (MeshFace){quadVerts[2], quadVerts[1], quadVerts[3]};
	}
	
	Mesh* newMesh = malloc(sizeof(Mesh));
	newMesh->vertCount = vertCount; newMesh->verts = newVerts; newMesh->faceCount = faceCount; newMesh->faces = newFaces; 
	return newMesh;
}

Mesh* genCylinderMesh(float btmRad, float topRad, float length, int res){
	if(fabs(btmRad) + fabs(topRad) == 0 || length == 0 || res < 3) return NULL;
	Uint32 vertCount = res * 2;
	Uint32 faceCount = 4 * res - 4;
	
	MeshVert* newVerts = malloc(sizeof(MeshVert) * vertCount);
	MeshFace* newFaces = malloc(sizeof(MeshFace) * faceCount);
	
	float angleRes = PI * 2 / res;
	for(int i=0; i<vertCount; i++){
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
		MeshVert *quadVerts[4] = {&newVerts[i], &newVerts[(i+1) % res], &newVerts[(i + res)], &newVerts[(i+1)%res + res]};
		
		newFaces[newI] = (MeshFace){quadVerts[0], quadVerts[1], quadVerts[2]};
		newFaces[(newI + 1)] = (MeshFace){quadVerts[2], quadVerts[1], quadVerts[3]};
	}
	
	for(int i=0; i<res-2; i++){
		int newI = 2 * res + i;
		newFaces[newI] = (MeshFace){&newVerts[i], &newVerts[0], &newVerts[(i + 1) % res]};
		newFaces[newI + res - 2] = (MeshFace){&newVerts[res], &newVerts[i + res], &newVerts[res + (i + 1) % res]};
	}
	
	Mesh* newMesh = malloc(sizeof(Mesh));
	newMesh->vertCount = vertCount; newMesh->verts = newVerts; newMesh->faceCount = faceCount; newMesh->faces = newFaces; 
	return newMesh;
}

Mesh* genPlaneMesh(float xScale, float yScale, int xRes, int yRes){
	if(fabs(xScale) + fabs(yScale) == 0 || abs(xRes) + abs(yRes) == 0) return NULL;
	
	Uint32 vertCount = (xRes + 1) * (yRes + 1);
	Uint32 faceCount = xRes * yRes * 2;
	
	MeshVert* newVerts = malloc(sizeof(MeshVert) * vertCount);
	MeshFace* newFaces = malloc(sizeof(MeshFace) * faceCount);
	
	for(int i=0; i<vertCount; i++){
		newVerts[i].pos = (Vector3){
			floor(i / (xRes + 1)) * (xScale / xRes), 
			0, 
			(i % (xRes + 1)) * (yScale / yRes),
		};
		newVerts[i].norm = (Vector3){0, 1, 0};
	}
	for(Uint32 i=0; i<faceCount/2; i++){
		Uint32 newI = i * 2;
		MeshVert *quadVerts[4] = {&newVerts[i], &newVerts[i+1], &newVerts[i + (xRes + 1)], &newVerts[i + 2 + xRes]};
		
		newFaces[newI] = (MeshFace){quadVerts[0], quadVerts[1], quadVerts[2]};
		newFaces[(newI + 1)] = (MeshFace){quadVerts[2], quadVerts[1], quadVerts[3]};
	}
	
	Mesh* newMesh = malloc(sizeof(Mesh));
	newMesh->vertCount = vertCount; newMesh->verts = newVerts; newMesh->faceCount = faceCount; newMesh->faces = newFaces; 
	return newMesh;
}