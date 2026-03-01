#include "studio.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "../instances.h"
#include "../renderer.h"
#include "../math.h"

#include <gamefile.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern ClientData client; 
extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
ButtonMap stuMouseButtons[3];
ButtonMap stuKeyList[STUDIOKEYBIND_MAX];

extern SDL_Window *studioWindow;
extern SDL_Renderer *studioRenderer;

extern Font studioFont;

Button* currColButton = NULL;

/*CharColour buttonColours[BUTTONCOLOUR_MAX] = { //what do you MEAN defined in main.c
	(CharColour){205, 208, 226, 255, 0, COLOURMODE_RGB},
	(CharColour){231, 234, 249, 255, 0, COLOURMODE_RGB},
	(CharColour){124, 128, 154, 255, 0, COLOURMODE_RGB},
	(CharColour){177, 179, 191, 255, 0, COLOURMODE_RGB},
	(CharColour){255, 255, 255, 255, 0, COLOURMODE_RGB}
};*/

Button newLableButton(void* func, char* text, SDL_FRect rect){
	return (Button){text, rect, INPUTTYPE_BUTTON, func, NULL, true, true, false, false, NULL, NULL};
}
Button newImageButton(void* func, SDL_Texture* image, SDL_FRect dest, SDL_FRect source){
	SDL_FRect* sourceRect = malloc(sizeof(SDL_FRect)); memcpy(sourceRect, &source, sizeof(SDL_FRect));
	return (Button){NULL, dest, INPUTTYPE_BUTTON, func, NULL, true, true, false, false, image, sourceRect};
}
Button newColourButton(CharColour* target, SDL_FRect rect){
	return (Button){NULL, rect, INPUTTYPE_COLOUR, NULL, target, true, true, false, false, NULL, NULL};
}

bool updateButton(Button* item){
	if(!item->enabled || (item->buttonType == INPUTTYPE_BUTTON && !item->pressed)) return 1;
	
	item->hover = (SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS) && (between(mousePos.x - item->rect.x, 0, item->rect.w) && between(mousePos.y - item->rect.y, 0, item->rect.h));
	if(item->hover){
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER));
		if(!stuMouseButtons[0].down){item->down = false; return 1;}
		if(item->down) return 1;
		item->down = true;

		switch(item->buttonType){
			case INPUTTYPE_COLOUR: if(currColButton == item) currColButton = NULL; else currColButton = item; break;
			default: item->pressed(item); break;
		}
	}
	
	return 0;
}

extern SDL_Texture* colourPickTex;
void drawColourPicker(SDL_Renderer* render, Button* item, CharColour* target){
	float normVal = 255 - max(max(target->r, target->g), target->b);
	SDL_SetTextureColorMod(colourPickTex, target->r + normVal, target->g + normVal, target->b + normVal);
	SDL_RenderTexture(render, colourPickTex, &(SDL_FRect){0, 0, 256, 256}, &(SDL_FRect){item->rect.x, item->rect.y - 128, 128, 128});
	SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
	SDL_RenderFillRect(render, &(SDL_FRect){item->rect.x, item->rect.y - 128, 4, 4});
}

void drawButton(SDL_Renderer* render, Button* item){
	if(item->image){
		SDL_FRect* rect = &(SDL_FRect){0, 0, item->image->w, item->image->h};
		if(item->imageSrc) rect = item->imageSrc;
		SDL_RenderTexture(render, item->image, rect, &item->rect);
		return;
	}

	if(item->buttonType == INPUTTYPE_COLOUR && item->target){
		CharColour* targetColour = item->target;
		SDL_SetRenderDrawColor(render, targetColour->r, targetColour->g, targetColour->b, 255);
		SDL_RenderFillRect(render, &(SDL_FRect){item->rect.x, item->rect.y, item->rect.w/2, item->rect.h});
		SDL_SetRenderDrawColor(render, targetColour->r * ((float)targetColour->a/255), targetColour->g * ((float)targetColour->a/255), targetColour->b * ((float)targetColour->a/255), 255);
		SDL_RenderFillRect(render, &(SDL_FRect){item->rect.x + item->rect.w/2, item->rect.y, item->rect.w/2, item->rect.h});
		if(item == currColButton)drawColourPicker(render, item, targetColour);
		return;
	}
	
	if(item->buttonType == INPUTTYPE_TEXT || item->buttonType == INPUTTYPE_NUMBER){
		setDrawColour(render, (CharColour){255, 255, 255, 255, 0, COLOURMODE_RGB});//buttonColours[BUTTONCOLOUR_INPUT]);
		goto buttonDrawStart;
	}
	
	if(!item->enabled){
		setDrawColour(render, (CharColour){177, 179, 191, 255, 0, COLOURMODE_RGB});//buttonColours[BUTTONCOLOUR_DISABLED]);
		goto buttonDrawStart;
	}
	setDrawColour(render, (CharColour){205, 208, 226, 255, 0, COLOURMODE_RGB});//buttonColours[BUTTONCOLOUR_DEFAULT]);
	if(item->hover)
		setDrawColour(render, (CharColour){231, 234, 249, 255, 0, COLOURMODE_RGB});//buttonColours[BUTTONCOLOUR_HOVER]);
	if(item->down)
		setDrawColour(render, (CharColour){124, 128, 154, 255, 0, COLOURMODE_RGB});//buttonColours[BUTTONCOLOUR_PRESSED]);
	
	buttonDrawStart:
	SDL_RenderFillRect(render, &item->rect);
	
	//SDL_SetRenderDrawColor(render, 0, 0, 0, SDL_ALPHA_OPAQUE);
	//SDL_RenderDebugText(render, item->rect.x + 2, item->rect.y + 2, item->labelText);
	drawText(render, &studioFont, item->labelText, item->rect.x + 2, item->rect.y + 2, 1, (SDL_FColor){0, 0, 0, 1});
}

void updateAndDrawButton(SDL_Renderer* render, Button* item){
	updateButton(item);
	drawButton(render, item);
}

extern DataType blockClass;
extern DataObj *focusObject;
extern DataType playerClass;

void buttonAddObject(Button* item){
	(void)item;
	DataObj *parentItem = focusObject;
	if(!focusObject) parentItem = client.gameWorld->headObj;
	DataObj *newItem = newObject(&blockClass);
	parentObject(newItem, parentItem);
	newItem->pos = (Vector3){floor(parentItem->pos.x) + 1, floor(parentItem->pos.y) + 1, floor(parentItem->pos.z) + 1};
	Vector3 normalizedColour = normalize3((Vector3){SDL_randf(), SDL_randf(), SDL_randf()});
	newItem->colour = (CharColour){normalizedColour.x * 255, normalizedColour.y * 255, normalizedColour.z * 255, 255, 0, COLOURMODE_RGB};
	
	if(newItem->classData == &blockClass){ //not piratesoftwaring here, add button object will have a dropdown of all the objects soon probably
		CollisionHull *newColl = malloc(sizeof(CollisionHull));
		newColl->shape = COLLHULL_CUBE; newItem->asVoidptr[OBJVAL_COLLIDER] = newColl;
	}
	
	parentItem->studioOpen = true;
	//focusObject = newItem;
}

void buttonRemoveObject(Button* item){
	(void)item;
	if(!focusObject) return;
	//if(focusObject == client.gameWorld->headObj) return;
	// if deleting the player, clear the reference (ofc)
	if(focusObject == client.gameWorld->currPlayer)
		client.gameWorld->currPlayer = NULL;
	removeObject(focusObject);
	focusObject = NULL;
}

static const SDL_DialogFileFilter mapLoadFilter[] = {
    {"JSON Map", "json"}, {"All File Types", "*"}
};

static void SDLCALL loadMapDialogue(void* userdata, const char* const* filelist, int filter){
	(void)userdata;
	if (!filelist) {
		printf("An error occured: %s\n", SDL_GetError());
		return;
	} else if (!*filelist) {
		printf("No file was selected.\n");
		return;
	}
        
	printf("Full path to selected file: '%s'\n", *filelist);
	
	//loadMapFromSBMap(*filelist);
	client.gameWorld->headObj->child = NULL;
	focusObject = NULL;
	
	if(loadGameFile(*filelist))
		sendPopup("Failed to load gamefile", NULL, NULL, 3);
	
	if (filter < 0) {
		printf("fuck!\n");
		return;
	} else if ((size_t)filter < SDL_arraysize(mapLoadFilter)) {
		printf("The filter selected by the user is '%s' (%s).\n", mapLoadFilter[filter].pattern, mapLoadFilter[filter].name);
		return;
	}
	
	//DataObj *playerObj = newObject(NULL, &playerClass);
	//client.gameWorld->currPlayer = playerObj;
}

static void SDLCALL saveMapDialogue(void* userdata, const char* const* filelist, int filter){
	(void)userdata;
	if (!filelist) {
		printf("An error occured: %s\n", SDL_GetError());
		return;
	} else if (!*filelist) {
		printf("No file was selected.\n");
		return;
	}
        
	printf("Full path to selected file: '%s'\n", *filelist);
	saveGameFile(*filelist);
	
	if (filter < 0) {
		printf("fuck!\n");
		return;
	} else if ((size_t)filter < SDL_arraysize(mapLoadFilter)) {
		printf("The filter selected by the user is '%s' (%s).\n", mapLoadFilter[filter].pattern, mapLoadFilter[filter].name);
		return;
	}
	
	//DataObj *playerObj = newObject(NULL, &playerClass);
	//client.gameWorld->currPlayer = playerObj;
}

void buttonLoadMap(Button* item){
	(void)item;
	SDL_ShowOpenFileDialog(loadMapDialogue, NULL, studioWindow, mapLoadFilter, SDL_arraysize(mapLoadFilter), SDL_GetCurrentDirectory(), false);
}

void buttonSaveMap(Button* item){
	(void)item;
	SDL_ShowSaveFileDialog(saveMapDialogue, NULL, studioWindow, mapLoadFilter, SDL_arraysize(mapLoadFilter), SDL_GetCurrentDirectory());
}

void buttonPauseGame(Button* item){
	item->imageSrc->x = 16 * client.pause;
	client.pause = !client.pause;
}

extern Uint32 toolMode;
void buttonSetTool(Button* item){
	toolMode = (int)floor(item->imageSrc->x / 16);
}

extern SDL_Window* window;
void StudioHandleKeys(){
	const bool* stuKeyState = SDL_GetKeyboardState(NULL);
	bool hasFocus = (SDL_GetWindowFlags(studioWindow) | SDL_GetWindowFlags(window)) & SDL_WINDOW_INPUT_FOCUS;
	for(int i = 0; i < STUDIOKEYBIND_MAX; i++){
		bool oldHeld = stuKeyList[i].down;
		stuKeyList[i].down = stuKeyState[stuKeyList[i].code] && hasFocus;
		stuKeyList[i].released = oldHeld && !stuKeyList[i].down;
		if(stuKeyList[i].down){
			if(!stuKeyList[i].pressCheck){
				stuKeyList[i].pressCheck = true;
				stuKeyList[i].pressed = true;
			}else{
				stuKeyList[i].pressed = false;
			}
		}else stuKeyList[i].pressCheck = false;
	}
}