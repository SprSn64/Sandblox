#include "studioInput.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern ClientData client; 
extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
extern ButtonMap stuMouseButtons[3];

extern SDL_Renderer *studioRenderer;

bool updateButton(Button* item){
	item->hover = (mousePos.x >= item->rect.x && mousePos.y >= item->rect.y && mousePos.x <= item->rect.x + item->rect.w && mousePos.y <= item->rect.y  + item->rect.h);
	if(item->pressed == NULL) return 1;
	//stuMouseButtons[0].down
	if(item->hover){
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER));
		//item->pressed();
		if(!stuMouseButtons[0].down){item->down = false;}else{
			if(!item->down){
				item->down = true;
				item->pressed();
			}
		}
	}
	
	return 0;
}

void drawButton(Button* item){
	SDL_SetRenderDrawColor(studioRenderer, 224 + 31 * item->hover, 224 + 31 * item->hover, 255, SDL_ALPHA_OPAQUE); 
	SDL_RenderFillRect(studioRenderer, &(SDL_FRect){item->rect.x, item->rect.y, item->rect.w, item->rect.h});
	
	SDL_SetRenderDrawColor(studioRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDebugText(studioRenderer, item->rect.x + 2, item->rect.y + 2, item->labelText);
}