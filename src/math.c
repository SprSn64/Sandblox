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

float *newMatrix(){
	float *output;
	output = calloc(0, sizeof(mat4));
	output[0] = 1; output[5] = 1; output[10] = 1; output[15] = 1; 
	return output;
}

Vector4 matrixMult(Vector4 vector, mat4 matrix){
	return (Vector4){
		vector.x * matrix[0] + vector.y * matrix[1] + vector.z * matrix[2] + vector.w * matrix[3], 
		vector.x * matrix[4] + vector.y * matrix[5] + vector.z * matrix[6] + vector.w * matrix[7],  
		vector.x * matrix[8] + vector.y * matrix[9] + vector.z * matrix[10] + vector.w * matrix[11], 
		vector.x * matrix[12] + vector.y * matrix[13] + vector.z * matrix[14] + vector.w * matrix[15], 
	};
}

/*matrix[0], matrix[1], matrix[2], matrix[3],
matrix[4], matrix[5], matrix[6], matrix[7],
matrix[8], matrix[9], matrix[10], matrix[11],
matrix[12], matrix[13], matrix[24], matrix[15],*/

float *scaleMatrix(mat4 matrix, Vector3 scale){
	float *output;
	output = malloc(sizeof(mat4));
	float tempMatrix[16] = {
		matrix[0] * scale.x, matrix[1], matrix[2], matrix[3],
		matrix[4], matrix[5] * scale.y, matrix[6], matrix[7],
		matrix[8], matrix[9], matrix[10] * scale.z, matrix[11],
		matrix[12], matrix[13], matrix[24], matrix[15],
	};
	
	memcpy(output, &tempMatrix, sizeof(float) * 16);
	return output;
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

float *axisRotMatrix(Uint8 axis, float angle){ //axis 0 = x (yz planes), axis 1 = y (xz planes), axis 2 = z (xy planes)
	float *output;
	output = malloc(sizeof(mat4));
	float angSin = SDL_sin(angle);
	float angCos = SDL_cos(angle);
	float tempMatrix[16] = {
		angCos * (axis != 0) + (axis == 0), -angSin * (axis == 2), angSin * (axis == 1), 0,
		angSin * (axis == 2), angCos * (axis != 1) + (axis == 1), -angSin * (axis == 0), 0,
		-angSin * (axis == 1), angSin * (axis == 0), angCos * (axis != 2) + (axis == 2), 0,
		0, 0, 0, 1,
	};
	memcpy(output, &tempMatrix, sizeof(float) * 16);
	return output;
}

float *rotateMatrix(mat4 matrix, Vector3 angle){
	float *output;
	output = malloc(sizeof(mat4));
	
	float *xMatrix = multMatrix(matrix, axisRotMatrix(0, angle.x));
	float *yMatrix = multMatrix(xMatrix, axisRotMatrix(1, angle.y));
	float *zMatrix = multMatrix(yMatrix, axisRotMatrix(2, angle.z));
	
	memcpy(output, &zMatrix, sizeof(float) * 16);
	return output;
}