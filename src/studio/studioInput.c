#include "studioInput.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "../instances.h"
#include "../math.h"
#include "../map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern ClientData client; 
extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
ButtonMap stuMouseButtons[3];
ButtonMap stuKeyList[5];

extern SDL_Window *studioWindow;
extern SDL_Renderer *studioRenderer;

bool updateButton(Button* item){
	item->hover = SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS && (mousePos.x >= item->rect.x && mousePos.y >= item->rect.y && mousePos.x <= item->rect.x + item->rect.w && mousePos.y <= item->rect.y  + item->rect.h);
	if(item->pressed == NULL) return 1;
	//stuMouseButtons[0].down
	if(item->hover){
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER));
		//item->pressed();
		if(!stuMouseButtons[0].down){item->down = false; return 1;}
		
		if(item->down) return 1;
		item->down = true;
		item->pressed(item);
	}
	
	return 0;
}

void drawButton(Button* item){
	SDL_SetRenderDrawColor(studioRenderer, 187, 187, 187, SDL_ALPHA_OPAQUE); 
	if(item->enabled)SDL_SetRenderDrawColor(studioRenderer, 224 + 31 * item->hover, 224 + 31 * item->hover, 255, SDL_ALPHA_OPAQUE); 
	SDL_RenderFillRect(studioRenderer, &(SDL_FRect){item->rect.x, item->rect.y, item->rect.w, item->rect.h});
	
	SDL_SetRenderDrawColor(studioRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDebugText(studioRenderer, item->rect.x + 2, item->rect.y + 2, item->labelText);
}

extern DataType blockClass;
extern DataObj *focusObject;
extern DataType playerClass;

void buttonAddObject(Button* item){
	DataObj *newItem = newObject(focusObject, &blockClass);
	newItem->pos = (Vector3){floor(focusObject->pos.x) + 1, floor(focusObject->pos.y) + 1, floor(focusObject->pos.z) + 1};
	Vector3 normalizedColour = normalize3((Vector3){SDL_randf(), SDL_randf(), SDL_randf()});
	newItem->colour = (CharColour){normalizedColour.x * 255, normalizedColour.y * 255, normalizedColour.z * 255, 255, 0, COLOURMODE_RGB};
	
	focusObject->studioOpen = true;
	//focusObject = newItem;
}

void buttonRemoveObject(Button* item){
	if(!focusObject) return;
	removeObject(focusObject);
	focusObject = NULL;
}

static const SDL_DialogFileFilter mapLoadFilter[] = {
    {"Sandblox Map", "sbmap"}
};

static void SDLCALL openMapDialogue(void* userdata, const char* const* filelist, int filter){
	if (!filelist) {
		printf("An error occured: %s\n", SDL_GetError());
		return;
	} else if (!*filelist) {
		printf("No file was selected.\n");
		return;
	}
        
	printf("Full path to selected file: '%s'\n", *filelist);
	client.gameWorld->headObj->child = NULL;
	loadMapFromSBMap(*filelist);
	
	DataObj *playerObj = newObject(NULL, &playerClass);
	client.gameWorld->currPlayer = playerObj;
	
	if (filter < 0) {
		printf("fuck!\n");
		return;
	} else if (filter < SDL_arraysize(mapLoadFilter)) {
		printf("The filter selected by the user is '%s' (%s).\n", mapLoadFilter[filter].pattern, mapLoadFilter[filter].name);
		return;
	}
}

void buttonLoadMap(Button* item){
	SDL_ShowOpenFileDialog(openMapDialogue, NULL, studioWindow, mapLoadFilter, SDL_arraysize(mapLoadFilter), SDL_GetCurrentDirectory(), false);
}

void StudioHandleKeys(){
	const bool* keyState = SDL_GetKeyboardState(NULL);
	for(int i = 0; i < 5; i++){
		stuKeyList[i].down = keyState[stuKeyList[i].code] && SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS;
	}
}