#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <structs.h>

typedef struct{
	Vector3 pos, norm;
	SDL_FPoint uv;
	SDL_FColor colour;
} MeshVert;

typedef struct{
	Uint32 vertA, vertB, vertC;
	//Material material; or something
} MeshFace;

typedef enum meshTypes{
	MESHTYPE_FILE,
	MESHTYPE_PLANE, MESHTYPE_SPHERE, MESHTYPE_TORUS, MESHTYPE_CYLINDER 
} meshTypes;

typedef struct{
	Uint32 vertCount;
	MeshVert *verts;
	Uint32 faceCount;
	MeshFace *faces;
	
	Uint32 meshType;
	char* filePath;
} Mesh;

typedef struct{
	SDL_Texture *image;
	Uint16 startChar; //32 starts at the space glyth
	SDL_Point glyphSize;
	SDL_FPoint kerning;
	Uint16 columns;
} Font;

SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode);
void drawText(SDL_Renderer *renderLoc, Font *textFont, char* text, short posX, short posY, float size, SDL_FColor colour);
void setDrawColour(SDL_Renderer *render, CharColour colour);

Vector3 worldToCamera(Vector3 pos);
Vector3 viewProj(Vector3 pos);
Vector3 projToScreen(Vector3 pos);

bool drawTriangle(MeshVert pointA, MeshVert pointB, MeshVert pointC, SDL_Texture* image);
bool draw3DTriangle(MeshVert pointA, MeshVert pointB, MeshVert pointC, SDL_Texture* image);

SDL_FColor ConvertSDLColour(CharColour colour);
CharColour ConvertColour(CharColour colour, Uint32 mode);

void drawMesh(Mesh* mesh, mat4 transform, SDL_FColor colour, SDL_Texture* texture, bool shaded);
void drawBillboard(SDL_Texture *texture, SDL_FRect rect, Vector3 pos, SDL_FPoint offset, SDL_FPoint scale);

Mesh* genTorusMesh(float outerRad, float innerRad, int ringRes, int ringCount);
Mesh* genCylinderMesh(float btmRad, float topRad, float length, int res);
Mesh* genPlaneMesh(float xScale, float yScale, int xRes, int yRes);

#endif