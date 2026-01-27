#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "structs.h"

float lerp(float a, float b, float t){return a + (b - a) * t;}
float invLerp(float a, float b, float v){ //gives the t value for getting a specific output from lerp()
	return (v - a) / (b - a);}

float dotProd2(SDL_FPoint vecA, SDL_FPoint vecB){return vecA.x * vecB.x + vecA.y * vecB.y;}
float dotProd3(Vector3 vecA, Vector3 vecB){return vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z;}

SDL_FPoint normalize2(SDL_FPoint vec){
	float length = sqrt(vec.x * vec.x + vec.y * vec.y);
	return (SDL_FPoint){vec.x / length, vec.y / length};
}
Vector3 normalize3(Vector3 vec){
	float length = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	return (Vector3){vec.x / length, vec.y / length, vec.z / length};
}
Vector3 reflect(Vector3 incident, Vector3 normal){
	float dot = dotProd3(normal, incident);
	return (Vector3){incident.x - 2 * dot * normal.x, incident.y - 2 * dot * normal.y, incident.z - 2 * dot * normal.z};
}
Vector3 vec3Add(Vector3 vecA, Vector3 vecB){
	return (Vector3){vecA.x + vecB.x, vecA.y + vecB.y, vecA.z + vecB.z};}
Vector3 vec3Mult(Vector3 vecA, Vector3 vecB){
	return (Vector3){vecA.x * vecB.x, vecA.y * vecB.y, vecA.z * vecB.z};}
	
Vector4 vec3ToVec4(Vector3 vec){return (Vector4){vec.x, vec.y, vec.z, 1};}
Vector3 vec4ToVec3(Vector4 vec){return (Vector3){vec.x * vec.w, vec.y * vec.w, vec.z * vec.w};}

Vector3 rotToNorm3(Vector3 rot){
	return (Vector3){SDL_cos(rot.x) * SDL_sin(rot.y), -SDL_sin(rot.x), SDL_cos(rot.x) * SDL_cos(rot.y)};}
	
Vector3 normToRot3(Vector3 norm){
	Vector3 normed = normalize3(norm);
	
	return (Vector3){atan2(normed.y, normed.x), atan2(sqrt(normed.x * normed.x + normed.y * normed.y), normed.z), 0};
}

float closest(float input, float snap){return floor(input / snap) * snap;}
bool between(float input, float min, float max){return(input >= min && input <= max);}

float *newMatrix(){
	float *output;
	output = calloc(1, sizeof(mat4));;
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
	float *output = malloc(sizeof(mat4));
	output[0] = matrix[0] * scale.x; output[1] = matrix[1]; output[2] = matrix[2]; output[3] = matrix[3];
	output[4] = matrix[4]; output[5] = matrix[5] * scale.y; output[6] = matrix[6]; output[7] = matrix[7];
	output[8] = matrix[8]; output[9] = matrix[9]; output[10] = matrix[10] * scale.z; output[11] = matrix[11];
	output[12] = matrix[12]; output[13] = matrix[13]; output[14] = matrix[14]; output[15] = matrix[15];
	return output;
}

float *translateMatrix(mat4 matrix, Vector3 move){
	float *output = malloc(sizeof(mat4));
	output[0] = matrix[0]; output[1] = matrix[1]; output[2] = matrix[2]; output[3] = matrix[3] + move.x;
	output[4] = matrix[4]; output[5] = matrix[5]; output[6] = matrix[6]; output[7] = matrix[7] + move.y;
	output[8] = matrix[8]; output[9] = matrix[9]; output[10] = matrix[10]; output[11] = matrix[11] + move.z;
	output[12] = matrix[12]; output[13] = matrix[13]; output[14] = matrix[14]; output[15] = matrix[15];
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
	output = malloc(sizeof(mat4));;
	float angSin = SDL_sin(angle);
	float angCos = SDL_cos(angle);
	float tempMatrix[16] = {
		1, 0, 0, 0, 
		0, 1, 0, 0, 
		0, 0, 1, 0, 
		0, 0, 0, 1
	};
	
	/* = {
		angCos * axis + !axis, -angSin * (axis == 2) + (axis != 2), angSin * (axis == 1) + (axis != 1), 1,
		angSin * (axis == 2) + (axis != 2), angCos * (axis != 1) + (axis == 1), -angSin * !axis + axis, 1,
		-angSin * (axis == 1) + (axis != 1), angSin * !axis + (axis), angCos * (axis != 2) + (axis == 2), 1,
		0, 0, 0, 1,
	};*/
	
	switch(axis){
		case 0: //x
			tempMatrix[5] = angCos; tempMatrix[6] = -angSin; tempMatrix[9] = angSin; tempMatrix[10] = angCos;
			break;
		case 1: //y
			tempMatrix[0] = angCos; tempMatrix[2] = angSin; tempMatrix[8] = -angSin; tempMatrix[10] = angCos;
			break;
		case 2: //z
			tempMatrix[0] = angCos; tempMatrix[1] = -angSin; tempMatrix[4] = angSin; tempMatrix[5] = angCos;
			break;
	}
	
	memcpy(output, &tempMatrix, sizeof(mat4));
	return output;
} // probably needs fixing

float *rotateMatrix(mat4 matrix, Vector3 angle){
	float *output;
	output = malloc(sizeof(mat4));
	
	float *xMatrix = multMatrix(matrix, axisRotMatrix(0, angle.x));
	float *yMatrix = multMatrix(xMatrix, axisRotMatrix(1, angle.y));
	float *zMatrix = multMatrix(yMatrix, axisRotMatrix(2, angle.z));
	
	memcpy(output, zMatrix, sizeof(mat4));
	free(xMatrix); free(yMatrix); free(zMatrix); 
	return output;
}

extern float* defaultMatrix;

float *genMatrix(Vector3 pos, Vector3 scale, Vector3 rot){
	float *output;
	output = malloc(sizeof(mat4));
	
	float *scaled = scaleMatrix(defaultMatrix, scale);
	float *rotated = rotateMatrix(scaled, rot);
	float *translated = translateMatrix(rotated, pos);
	
	memcpy(output, translated, sizeof(mat4));
	free(translated); free(scaled); free(rotated); 
	return output;
}

//versions of above functions but without generating new ones

void translateMatrix2(mat4 matrix, Vector3 move){
	matrix[3] += move.x;
	matrix[7] += move.y;
	matrix[11] += move.z;
}

void scaleMatrix2(mat4 matrix, Vector3 scale){
	matrix[0] *= scale.x;
	matrix[5] *= scale.y;
	matrix[10] *= scale.z;
}

void rotateMatrix2(mat4 matrix, Vector3 angle){
	matrix[0] *= SDL_cos(angle.y) * SDL_cos(angle.z); matrix[1] += -SDL_sin(angle.z); matrix[2] += SDL_sin(angle.y);
	matrix[5] *= SDL_cos(angle.x); matrix[6] += -SDL_sin(angle.x);
	matrix[8] += -SDL_sin(angle.y); matrix[9] += SDL_sin(angle.x); matrix[10] *= SDL_cos(angle.x) * SDL_cos(angle.y);
}

float *perspMatrix(float fov, float aspect, float zNear, float zFar){
	//bool rORl = false;
	//bool ZOorNO = true;
	float halfFov = tan(fov / 2);
	float *output = newMatrix();

	output[0] = 1 / (halfFov * aspect);
	output[5] = 1 / halfFov;
	output[11] = 1;// - 2 * rORl;
	/*if (!ZOorNO)
	{
		output[10] = zFar / (zNear - zFar);
		output[14] = -(zFar * zNear) / (zNear - zFar);
	}
	else
	{*/
		output[10] = -(zFar + zNear) / (zNear - zFar);
		output[14] = -(2 * zFar * zNear) / (zNear - zFar);
	//}

	return output;
}

Vector3 extractTranslation(mat4 matrix){
	return (Vector3){matrix[3], matrix[7], matrix[11]};
}

Vector3 extractScale(mat4 matrix){
	float scaleX = sqrt(matrix[0] * matrix[0] + matrix[4] * matrix[4] + matrix[8] * matrix[8]);
	float scaleY = sqrt(matrix[1] * matrix[1] + matrix[5] * matrix[5] + matrix[9] * matrix[9]);
	float scaleZ = sqrt(matrix[2] * matrix[2] + matrix[6] * matrix[6] + matrix[10] * matrix[10]);
	return (Vector3){scaleX, scaleY, scaleZ};
}

void extractRotMatrix(mat4 matrix, mat4 outputLoc){
	Vector3 scale = extractScale(matrix);
	float output[16] = {
		matrix[0] / scale.x, matrix[1] / scale.y, matrix[2] / scale.z, 0,
		matrix[4] / scale.x, matrix[5] / scale.y, matrix[6] / scale.z, 0,
		matrix[8] / scale.x, matrix[9] / scale.y, matrix[10] / scale.z, 0,
		0, 0, 0, 1
	};
	memcpy(outputLoc, output, sizeof(mat4));
}