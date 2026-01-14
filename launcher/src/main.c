#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//do something with linux support or something here
#include <windows.h>

#include <structs.h>
#include "input.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Point windowScale = {640, 480};

Uint64 last = 0;
Uint64 now = 0;
double deltaTime = 0;
Uint32 lastFPS = 0;

float timer = 0;

#define KEYBINDCOUNT 2
ButtonMap keyList[KEYBINDCOUNT];

bool windowHover = false;

SDL_MouseButtonFlags mouseState;
SDL_FPoint mousePos;
ButtonMap mouseButtons[3];
void HandleKeyInput();

char *clientLoc = "..//sandblox.exe";

Button launchButton = {"launch", (SDL_FRect){224, 304, 64, 16}, buttonLaunch, true, true, false, false};

SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode);
void drawText(SDL_Renderer *renderer, SDL_Texture *texture, char* text, char charOff, short posX, short posY, short width, short height, short kern);

SDL_Texture* fontTex = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	SDL_SetAppMetadata("SandBlox", "0.0", NULL);
	
	for(int i=0; i < argc; i++){
		//printf("%s\n", argv[i]);
	}
	
	if(!SDL_Init(SDL_INIT_VIDEO)){
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if(!SDL_CreateWindowAndRenderer("Sandblox Launcher 0.0", windowScale.x, windowScale.y, SDL_WINDOW_RESIZABLE * 0, &window, &renderer)){
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetWindowMinimumSize(window, 320, 240);
	SDL_SetRenderVSync(renderer, 1);
	
	fontTex = newTexture("assets/font.png", 0);

	return SDL_APP_CONTINUE;
}	

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
	if(event->type == SDL_EVENT_QUIT){
		return SDL_APP_SUCCESS;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){
	HandleKeyInput();
	
	windowHover = SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS;
	SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT));
	
	mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
	for(int i=0; i<3; i++){
		mouseButtons[i].down = (windowHover && (mouseState & mouseButtons[i].code));
		if(mouseButtons[i].down){
			if(!mouseButtons[i].pressCheck){
				mouseButtons[i].pressCheck = true;
				mouseButtons[i].pressed = true;
			}else{
				mouseButtons[i].pressed = false;
			}
		}else mouseButtons[i].pressCheck = false;
	}
	
	last = now;
	now = SDL_GetTicks();
	deltaTime = min(((double)now - (double)last) / 1000.0f, 1);
	timer += deltaTime;
	
	SDL_GetWindowSize(window, &windowScale.x, &windowScale.y);
	//windowScaleFactor = min((float)windowScale.x / windowScaleIntent.x, (float)windowScale.y / windowScaleIntent.y);
	
	SDL_SetRenderDrawColor(renderer, 20, 22, 24, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	
	updateButton(&launchButton); drawButton(&launchButton);
	
	//drawText(renderer, fontTex, "johnsons", 32, 0, 0, 16, 16, 14);

	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
	SDL_DestroyTexture(fontTex);
}
    
void HandleKeyInput(){
	const bool* keyState = SDL_GetKeyboardState(NULL);
	for(int i = 0; i < KEYBINDCOUNT; i++){
		keyList[i].down = keyState[keyList[i].code] && SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS;
		if(keyList[i].down){
			if(!keyList[i].pressCheck){
				keyList[i].pressCheck = true;
				keyList[i].pressed = true;
			}else{
				keyList[i].pressed = false;
			}
		}else keyList[i].pressCheck = false;
	}
}

SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode){
	SDL_Texture *texture = IMG_LoadTexture(renderer, path);
	if(texture == NULL){
		printf("Issue with loading texture %s!\n", path);
		return NULL;
	}
	SDL_SetTextureScaleMode(texture, scaleMode);
	return texture;
}

void drawText(SDL_Renderer *renderer, SDL_Texture *texture, char* text, char charOff, short posX, short posY, short width, short height, short kern){
	for(size_t i=0; i<=strlen(text); i++){
		char charVal = (unsigned)text[i] - charOff;
		int xOff = (charVal % 16) * width;
		int yOff = floor((float)charVal / 16) * height;
		SDL_FRect sprRect = {xOff, yOff, width, height};
		SDL_FRect sprPos = {posX + kern * i, posY, width, height};
		SDL_RenderTexture(renderer, texture, &sprRect, &sprPos);
	}
}