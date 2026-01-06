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
extern ButtonMap mouseButtons[3];

bool updateButton(Button* item){
	item->hover = (mousePos.x >= item->rect.x && mousePos.y >= item->rect.y && mousePos.x <= item->rect.x + item->rect.w && mousePos.y <= item->rect.y  + item->rect.h);
	if(item->pressed == NULL) return 1;
	//mouseButtons[0]
	if(item->hover){
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER));
		//item->pressed();
		if(!mouseButtons[0].down){item->down = false;}else{
			if(!item->down){
				item->down = true;
				item->pressed();
			}
		}
	}
	
	return 0;
}