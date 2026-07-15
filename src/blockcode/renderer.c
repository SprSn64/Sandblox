#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <structs.h>
#include "structs.h"
#include "renderer.h"

#include "../math.h"
#include "../opengl.h"

extern Mesh* planePrim;
extern float* defaultMatrix;
void drawCodeBlock(CodeBlock* item, SDL_FPoint pos){
	if(!item->classItem) return;

	float* blockMatrix = genMatrix((Vector3){pos.x, pos.y, 0}, (Vector3){0.2, 1, 0.05}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, blockMatrix, item->classItem->colour, NULL);
	free(blockMatrix);
}