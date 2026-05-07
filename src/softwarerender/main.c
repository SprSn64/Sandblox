#include "main.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../math.h"
#include "../renderer.h"

Uint32 colourToInt(SDL_FColor colour){
	return (int)(colour.r * 255) + ((int)(colour.g * 255) << 8) + ((int)(colour.b * 255) << 16) + ((int)(colour.a * 255) << 24); 
}

SDL_FColor intToColour(Uint32 colour){
	return (SDL_FColor){
		(float)(colour & 0x000000FF) / 255, 
		(float)((colour & 0x0000FF00) >> 8) / 255,
		(float)((colour & 0x00FF0000) >> 16) / 255,
		(float)((colour & 0xFF000000) >> 24) / 255
	};
}

Uint32 colourLerp(Uint32 colA, Uint32 colB, float t){
	SDL_FColor newColA = intToColour(colA); SDL_FColor newColB = intToColour(colB); 
	return colourToInt((SDL_FColor){lerp(newColA.r, newColB.r, t), lerp(newColA.g, newColB.g, t), lerp(newColA.b, newColB.b, t), lerp(newColA.a, newColB.a, t)});
}

void setPixel(Texture* target, Uint16 x, Uint16 y, Uint32 colour){
	if(!target || x >= target->width || y >= target->height || (colour & 0xFF000000) >> 24 == 0) return;
	if((colour & 0xFF000000) >> 24 != 0xFF){
		float alpha = (float)((colour & 0xFF000000) >> 24) / 255;
		target->pixels[x + y * target->width] = colourLerp(target->pixels[x + y * target->width], colour, alpha);
		//newColour.a = min(colourGot.a + colour.a, 1);
		return;
	}
	target->pixels[x + y * target->width] = colour;
}

Uint32 getPixel(Texture* target, Uint16 x, Uint16 y){
	if(!target) return 0x00000000;
	return target->pixels[(x % target->width) + (y % target->height) * target->width];
}

void clearTex(Texture* target, Uint32 colour){
	for(int i=0; i<target->width * target->height; i++){
		target->pixels[i] = colour;
	}
}

void drawTexture(Texture* target, Texture* tex, SDL_Rect* source, SDL_Rect* dest, Uint32 colour){
	(void)colour;
	if(!tex || !target) return;
	for(Uint32 i=0; i<(Uint32)abs(dest->w) * abs(dest->h); i++){
		Uint32 currPixel = getPixel(tex, 
			source->x + (i % abs(dest->w)) * ((float)source->w/dest->w) + (source->w - 1) * (dest->w < 0), 
			source->y + (i / abs(dest->w)) * ((float)source->h/dest->h) + (source->h - 1) * (dest->h < 0)
		);
		if(currPixel >> 24 > 0)
			setPixel(target, dest->x + i % abs(dest->w), dest->y + (i / abs(dest->w)), currPixel);
	}
}

void drawRasterText(Texture* target, Font *textFont, char* text, short posX, short posY, float scale, Uint32 colour){
	if(!target || !textFont) return;
	for(size_t i=0; i<strlen(text); i++){
		char charVal = text[i] - textFont->startChar;
		int xOff = (charVal % textFont->columns) * textFont->glyphSize.x;
		int yOff = charVal / textFont->columns * textFont->glyphSize.y;
		SDL_Rect sprRect = {xOff, yOff, textFont->glyphSize.x, textFont->glyphSize.y};
		SDL_Rect sprPos = {posX + textFont->kerning.x * i * scale, posY + textFont->kerning.y * i * scale, textFont->renderSize.x * scale, textFont->renderSize.y * scale};
		drawTexture(target, textFont->rastTex, &sprRect, &sprPos, colour);
	}
}

void drawRect(Texture* target, Uint16 posX, Uint16 posY, Uint16 width, Uint16 height, Uint32 colour){
	if(!target) return;
	for(Uint32 i=0; i<(Uint32)width * height; i++){
		setPixel(target, posX + i % width, posY + (i / width), colour);
	}
}

void drawHamLine(Texture* target, SDL_Point pointA, SDL_Point pointB, Uint32 colour){
	if(abs(pointB.x - pointA.x) > abs(pointB.y - pointA.y)){
		SDL_Point delta = {abs(pointB.x - pointA.x), pointB.y - pointA.y}; 
		Sint8 dirX = 1 - 2 * (pointA.x > pointB.x);
		Sint8 dirY = 1 - 2 * (delta.y < 0);
		delta.y = delta.y * dirY;
		if(delta.x == 0) return;
		int decide = 2 * delta.y - delta.x;
		int newY = pointA.y;
		for(int i=0; i <= delta.x; i++){
			setPixel(target, pointA.x + i * dirX, newY, colour);
			if(decide >= 0){
				newY+=dirY;
				decide += -2*delta.x;
			}
			decide += 2*delta.y;
		}
	}else{
		SDL_Point delta = {pointB.x - pointA.x, abs(pointB.y - pointA.y)}; 
		Sint8 dirY = 1 - 2 * (pointA.y > pointB.y);
		Sint8 dirX = 1 - 2 * (delta.x < 0);
		delta.x = delta.x * dirX;
		if(delta.y == 0) return;
		int decide = 2 * delta.x - delta.y;
		int newX = pointA.x;
		for(int i=0; i <= delta.y; i++){
			setPixel(target, newX, pointA.y + i * dirY, colour);
			if(decide >= 0){
				newX+=dirX;
				decide += -2*delta.y;
			}
			decide += 2*delta.x;
		}
	}
	return;
}