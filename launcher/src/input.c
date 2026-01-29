#include "input.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern bool windowHover;
extern SDL_FPoint mousePos;
extern ButtonMap mouseButtons[3];

extern char *clientLoc;

bool updateButton(Button* item){
	if(!item->enabled || !item->pressed) return 1;
	
	item->hover = SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS && (mousePos.x >= item->rect.x && mousePos.y >= item->rect.y && mousePos.x <= item->rect.x + item->rect.w && mousePos.y <= item->rect.y  + item->rect.h);
	if(item->hover){
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER));
		//item->pressed();
		if(!mouseButtons[0].down){item->down = false; return 1;}
		
		if(item->down) return 1;
		item->down = true;
		item->pressed(item);
	}
	
	return 0;
}

void drawButton(Button* item){
	SDL_SetRenderDrawColor(renderer, 187, 187, 187, SDL_ALPHA_OPAQUE); 
	if(item->enabled)SDL_SetRenderDrawColor(renderer, 224 + 31 * item->hover, 224 + 31 * item->hover, 255, SDL_ALPHA_OPAQUE); 
	SDL_RenderFillRect(renderer, &(SDL_FRect){item->rect.x, item->rect.y, item->rect.w, item->rect.h});
	
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDebugText(renderer, item->rect.x + 2, item->rect.y + 2, item->labelText);
}

void buttonLaunch(Button* item){
	printf("fuck");
	item->enabled = false;
	
	//run client
	
	#ifdef _WIN32
	FILE *file = fopen("config.txt", "r");
	if(!file){
		printf("Couldnt find config file\n");
		return;
	}
	
	char gameDir[512];
	fgets(gameDir, 512, file);
	printf("Game directory is at %s.\n", gameDir);
	fclose(file);
	
	/*CreateProcessA(
		gameDir,
		[in, out, optional] LPSTR                 lpCommandLine,
		[in, optional]      LPSECURITY_ATTRIBUTES lpProcessAttributes,
		[in, optional]      LPSECURITY_ATTRIBUTES lpThreadAttributes,
		[in]                BOOL                  bInheritHandles,
		[in]                DWORD                 dwCreationFlags,
		[in, optional]      LPVOID                lpEnvironment,
		[in, optional]      LPCSTR                lpCurrentDirectory,
		[in]                LPSTARTUPINFOA        lpStartupInfo,
		[out]               LPPROCESS_INFORMATION lpProcessInformation
	);*/
	#endif

	#ifdef linux
		
	#endif
}