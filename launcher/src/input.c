#include "input.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>

/*
_STARTUPINFOA {
  DWORD  cb;
  LPSTR  lpReserved;
  LPSTR  lpDesktop;
  LPSTR  lpTitle;
  DWORD  dwX;
  DWORD  dwY;
  DWORD  dwXSize;
  DWORD  dwYSize;
  DWORD  dwXCountChars;
  DWORD  dwYCountChars;
  DWORD  dwFillAttribute;
  DWORD  dwFlags;
  WORD   wShowWindow;
  WORD   cbReserved2;
  LPBYTE lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;

typedef struct _PROCESS_INFORMATION {
  HANDLE hProcess;
  HANDLE hThread;
  DWORD  dwProcessId;
  DWORD  dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;
*/

#endif

extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern Font defaultFont;
void drawText(SDL_Renderer *renderLoc, Font *textFont, char* text, short posX, short posY, float scale, SDL_FColor colour);

extern bool windowHover;
extern SDL_FPoint mousePos;
extern ButtonMap mouseButtons[3];

extern char *clientLoc;
extern char *basePath;

char *runArgs = "-studio";

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

void drawButton(SDL_Renderer* render, Button* item){
	/*if(item->image){
		SDL_FRect* rect = &(SDL_FRect){0, 0, item->image->w, item->image->h};
		if(item->imageSrc) rect = item->imageSrc;
		SDL_RenderTexture(render, item->image, rect, &item->rect);
		return;
	}*/

	if(!item->enabled){
		SDL_SetRenderDrawColor(render, 177, 179, 191, 255);//buttonColours[BUTTONCOLOUR_DISABLED]);
		goto buttonDrawStart;
	}
	SDL_SetRenderDrawColor(render, 205, 208, 226, 255);//buttonColours[BUTTONCOLOUR_DEFAULT]);
	if(item->hover)
		SDL_SetRenderDrawColor(render, 231, 234, 249, 255);//buttonColours[BUTTONCOLOUR_HOVER]);
	if(item->down)
		SDL_SetRenderDrawColor(render, 124, 128, 154, 255);//buttonColours[BUTTONCOLOUR_PRESSED]);
	
	buttonDrawStart:
	SDL_RenderFillRect(render, &(SDL_FRect){item->rect.x, item->rect.y, item->rect.w, item->rect.h});
	
	SDL_SetRenderDrawColor(render, 0, 0, 0, SDL_ALPHA_OPAQUE);
	drawText(render, &defaultFont, item->labelText, item->rect.x + 2, item->rect.y + 2, 1, (SDL_FColor){0, 0, 0, 1});
}

void buttonLaunch(Button* item){
	item->enabled = false;

	char configDir[512]; sprintf(configDir, "%s/config.cfg", basePath);
	FILE *file = fopen(configDir, "r");
	if(!file){
		printf("Couldnt find config file\n");
		goto reenableButton;
	}
	
	char gameDir[512]; bool fget = fgets(gameDir, 512, file); (void)fget;
	char ogDir[512]; sprintf(ogDir, "%s", SDL_GetCurrentDirectory());
	printf("Game directory is at %s.\n", gameDir);
	fclose(file);

	char command[2048];
	
#ifdef _WIN32
	SetCurrentDirectory(gameDir);
	sprintf(command, "sandblox.exe %s", runArgs);
	int systReturn = system(command);
	SetCurrentDirectory(ogDir);

#endif // _WIN32

#ifdef __linux__
#ifdef __x86_64__
			char *arch = "x86_64";
#elif defined(__i386__)
			char *arch = "x86";
#elif defined(__aarch64__) || defined(__arm64__)
			char *arch = "aarch64";
#elif defined(__arm__)
			char *arch = "arm";
#endif
		sprintf(command, "cd %s \n ./sandblox.%s %s \n cd %s", gameDir, arch, runArgs, ogDir);

		int systReturn = system(command);
#endif // __linux__
		printf("Program Output: %d\n", systReturn);
reenableButton:
	item->enabled = true;
}

static void SDLCALL getClientDialogue(void* userdata, const char* const* filelist, int filter){
	(void)userdata; (void)filter;
	if (!filelist) {
		printf("An error occured: %s\n", SDL_GetError());
		return;
	} else if (!*filelist) {
		printf("No file was selected.\n");
		return;
	}
        
	printf("Full path to selected file: '%s'\n", *filelist);

	char configDir[512]; sprintf(configDir, "%s/config.cfg", basePath);
	FILE *file = fopen(configDir, "w");
	fprintf(file, "%s", *filelist);
	fclose(file);
}

void buttonSelectClient(Button* item){
	(void)item;
	SDL_ShowOpenFolderDialog(getClientDialogue, NULL, window, SDL_GetCurrentDirectory(), false);
}

extern Uint32 currPage;
extern Button* sidePanelButtons;
void buttonSetPage(Button* item){
	for(int i=0; i<3; i++){
		if(item == &sidePanelButtons[i]){currPage=i; return;}
	}
}