#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <structs.h>

typedef struct{
	Vector3 pos, norm;
	SDL_FPoint uv;
	CharColour colour;
} MeshVert;

typedef struct{
	MeshVert *vertA, *vertB, *vertC;
	//Material material; or something
} MeshFace;

typedef struct{
	Uint32 vertCount;
	MeshVert *verts;
	Uint32 faceCount;
	MeshFace *faces;
} Mesh;

typedef struct SortTri{
	Vector3 vertA, vertB, vertC;
	SDL_FColor colour;
	struct SortTri *next;
} SortTri;

SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode);
void drawText(SDL_Renderer *renderer, SDL_Texture *texture, char* text, char charOff, short posX, short posY, short width, short height, short kern);

bool addListTri(Vector3 pointA, Vector3 pointB, Vector3 pointC, SDL_FColor colour);
void renderTriList();

Vector3 worldToCamera(Vector3 pos);

Vector3 isoProj(Vector3 posA);
Vector3 viewProj(Vector3 pos);
Vector3 projToScreen(Vector3 pos);

bool drawTriangle(Vector3 pointA, Vector3 pointB, Vector3 pointC, SDL_FColor colour);
bool draw3DTriangle(Vector3 pointA, Vector3 pointB, Vector3 pointC, SDL_FColor colour);

SDL_FColor charColConv(CharColour colour);

void drawCube(Vector3 pos, Vector3 scale, SDL_FColor colour);
void drawMesh(Mesh* mesh, mat4 transform, SDL_FColor colour);
void drawBillboard(SDL_Texture *texture, SDL_FRect rect, Vector3 pos, SDL_FPoint offset, SDL_FPoint scale);

#endif