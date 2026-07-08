#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "structs.h"
#include "renderer.h"
#include "math.h"
#include "opengl.h"

#include "softwarerender/main.h"
#include "softwarerender/depth.h"

extern SDL_Renderer *renderer;

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
extern float aspectRatio;
Vector3 screenToGL(Vector3 pos){
	return (Vector3){(pos.x / windowScale.x - 0.5) * 2 * aspectRatio, -pos.y / windowScale.y * 2 + 1, pos.z};
}

Vector3 glToScreen(Vector3 pos){
	return (Vector3){(pos.x + 1) / 2 * windowScale.x, (-pos.y + 1) / 2 * windowScale.y, pos.z};
}

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

void setDrawColour(SDL_Renderer *render, CharColour colour){
	SDL_SetRenderDrawColor(render, colour.r, colour.g, colour.b, colour.a); 
}