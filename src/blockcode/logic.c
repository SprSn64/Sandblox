#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <structs.h>
#include "structs.h"
#include "logic.h"

#include "../instances.h"
#include "../math.h"

void codeblock_print(CodeBlock* block){
	if(!block->content){
		char* logText = malloc(64); sprintf(logText, "why print nothing... shaking my head!");
		logToConsole(logText, CONSOLELOG_DEFAULT); 
		return;
	}
	
	char* logText = strdup(block->content);
	logToConsole(logText, CONSOLELOG_DEFAULT);
}

void codeblock_fRand(CodeBlock* block){
	if(block->output)
		free(block->output);
	float* randFloat = malloc(sizeof(float));
	block->output = randFloat;

	*randFloat = fRand(0, 1);
}