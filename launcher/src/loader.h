#ifndef LOADER_H
#define LOADER_H

#include <SDL3/SDL.h>

typedef enum languageItems{
	LANG_EN, LANG_ptBR, LANG_ES, LANG_DE, LANG_JP
} languageItems;

bool loadMapDir(char* path);
bool loadLanguage(Uint32 lang);

#endif
