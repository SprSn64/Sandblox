#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "structs.h"

float lerp(float a, float b, float t){
	return a + (b - a) * t;
}

float invLerp(float a, float b, float v){ //gives the t value for getting a specific output from lerp()
	return (v - a) / (b - a);
}

float dotProd2(SDL_FPoint vecA, SDL_FPoint vecB){
	return vecA.x * vecB.x + vecA.y * vecB.y;
}

float dotProd3(Vector3 vecA, Vector3 vecB){
	return vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z;
}

float *multMatrix(mat4 matrixA, mat4 matrixB){
	// horrible code warning
	float *output;
	output = malloc(sizeof(mat4));

	output[0] = matrixA[0] * matrixB[0] + matrixA[4] * matrixB[1] + matrixA[8] * matrixB[2] + matrixA[12] * matrixB[3];
	output[1] = matrixA[1] * matrixB[0] + matrixA[5] * matrixB[1] + matrixA[9] * matrixB[2] + matrixA[13] * matrixB[3];
	output[2] = matrixA[2] * matrixB[0] + matrixA[6] * matrixB[1] + matrixA[10] * matrixB[2] + matrixA[14] * matrixB[3];
	output[3] = matrixA[3] * matrixB[0] + matrixA[7] * matrixB[1] + matrixA[11] * matrixB[2] + matrixA[15] * matrixB[3];
	output[4] = matrixA[0] * matrixB[4] + matrixA[4] * matrixB[5] + matrixA[8] * matrixB[6] + matrixA[12] * matrixB[7];
	output[5] = matrixA[1] * matrixB[4] + matrixA[5] * matrixB[5] + matrixA[9] * matrixB[6] + matrixA[13] * matrixB[7];
	output[6] = matrixA[2] * matrixB[4] + matrixA[6] * matrixB[5] + matrixA[10] * matrixB[6] + matrixA[14] * matrixB[7];
	output[7] = matrixA[3] * matrixB[4] + matrixA[7] * matrixB[5] + matrixA[11] * matrixB[6] + matrixA[15] * matrixB[7];
	output[8] = matrixA[0] * matrixB[8] + matrixA[4] * matrixB[9] + matrixA[8] * matrixB[10] + matrixA[12] * matrixB[11];
	output[9] = matrixA[1] * matrixB[8] + matrixA[5] * matrixB[9] + matrixA[9] * matrixB[10] + matrixA[13] * matrixB[11];
	output[10] = matrixA[2] * matrixB[8] + matrixA[6] * matrixB[9] + matrixA[10] * matrixB[10] + matrixA[14] * matrixB[11];
	output[11] = matrixA[3] * matrixB[8] + matrixA[7] * matrixB[9] + matrixA[11] * matrixB[10] + matrixA[15] * matrixB[11];
	output[12] = matrixA[0] * matrixB[12] + matrixA[4] * matrixB[13] + matrixA[8] * matrixB[14] + matrixA[12] * matrixB[15];
	output[13] = matrixA[1] * matrixB[12] + matrixA[5] * matrixB[13] + matrixA[9] * matrixB[14] + matrixA[13] * matrixB[15];
	output[14] = matrixA[2] * matrixB[12] + matrixA[6] * matrixB[13] + matrixA[10] * matrixB[14] + matrixA[14] * matrixB[15];
	output[15] = matrixA[3] * matrixB[12] + matrixA[7] * matrixB[13] + matrixA[11] * matrixB[14] + matrixA[15] * matrixB[15];

	return output;
}