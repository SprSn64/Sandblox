#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <structs.h>
#include "blockcode.h"
#include "renderer.h"

void drawCodeBlock(SDL_Renderer* render, CodeBlock* item){
	if(!item->classItem) return;
	SDL_SetRenderDrawColor(render, item->classItem->colour.r, item->classItem->colour.g, item->classItem->colour.b, 255);
	SDL_RenderFillRect(render, &(SDL_FRect){item->pos.x, item->pos.y, 64, 12});
}