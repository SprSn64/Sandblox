#ifndef BLOCKCODE_INIT_H
#define BLOCKCODE_INIT_H

typedef struct cBlockClass{
	Uint32 id;
	SDL_FColor colour;

	Uint8 flags; //capped, operation block, pull thing, etc
} cBlockClass;

typedef struct CodeBlock{
	//class type (not sure if Uint32 or struct is better option)
	cBlockClass* classItem;
	SDL_FPoint pos;

	struct CodeBlock* prev;
	struct CodeBlock* next;
	struct CodeBlock* child;
	struct CodeBlock* parent;
} CodeBlock;

#include <structs.h>
#include "logic.h"
#include "renderer.h"

cBlockClass testBlockClass = {666, (SDL_FColor){1, 0, 0, 1}, 0};

#endif