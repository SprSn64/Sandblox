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

typedef struct MeshMaterial{
	//todo add colors?
	float specular;

	char name[128];
	char tex[256];
	TextureRef* texture;
} MeshMaterial;

typedef struct{
	Uint32 vertA, vertB, vertC;
	MeshMaterial *material;
} MeshFace;

typedef enum meshTypes{
	MESHTYPE_FILE,
	MESHTYPE_PLANE, MESHTYPE_SPHERE, MESHTYPE_TORUS, MESHTYPE_CYLINDER 
} meshTypes;

typedef struct Mesh{
	Uint32 vertCount;
	MeshVert *verts;
	Uint32 faceCount;
	MeshFace *faces;
	
	Uint32 vertArray, vertBuffer, eleBuffer;

	MeshMaterial *materials;

	Uint32 meshType;
	char* filePath;
	struct Mesh *prev;
	struct Mesh *next;
	bool persistent;
} Mesh;

typedef struct Texture{
	Uint32* pixels;
	Uint16 width, height;
} Texture;

typedef struct TextureRef{
	SDL_Texture *image;
	Texture* texture;
	char* filePath;

	Uint32 glLoc;
	struct TextureRef *prev;
	struct TextureRef *next; //for cleaning up and checking if texture already exists
	bool persistent;
} TextureRef;

typedef enum lightTypes{
	LIGHTSOURCE_SUN, LIGHTSOURCE_POINT
} lightTypes;

typedef struct{
	Uint32 lightType;
	Vector3 pos, rot, normal;
	SDL_FColor colour, specular;
	float strength, specStrength;
} LightSource;

typedef struct{
	TextureRef *texture;
	void *rastTex;
	Uint16 startChar; //32 starts at the space glyth
	SDL_Point glyphSize;
	SDL_Point renderSize;
	SDL_FPoint kerning;
	Uint16 columns;
} Font;

Vector3 glToScreen(Vector3 pos);
Vector3 screenToGL(Vector3 pos);

void drawText(SDL_Renderer *renderLoc, Font *textFont, char* text, short posX, short posY, float size, SDL_FColor colour);
void setDrawColour(SDL_Renderer *render, CharColour colour);
SDL_FColor clampColour(SDL_FColor colour);

SDL_FColor ConvertSDLColour(CharColour colour);
CharColour ConvertColour(CharColour colour, Uint32 mode);

void drawBillboard(TextureRef *texture, SDL_FRect rect, Vector3 pos, SDL_FPoint offset, SDL_FPoint scale);

Mesh* genTorusMesh(float outerRad, float innerRad, Uint16 ringRes, Uint16 ringCount);
Mesh* genCylinderMesh(float btmRad, float topRad, float length, int res);
Mesh* genPlaneMesh(float xScale, float yScale, Uint16 xRes, Uint16 yRes);

#endif