#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <structs.h>
#include "structs.h"
#include "renderer.h"

void drawCodeBlock(SDL_Renderer* render, CodeBlock* item){
	if(!item->classItem) return;
	SDL_SetRenderDrawColor(render, item->classItem->colour.r * 255, item->classItem->colour.g * 255, item->classItem->colour.b * 255, 255);
	SDL_RenderFillRect(render, &(SDL_FRect){item->pos.x, item->pos.y, 64, 12});
}