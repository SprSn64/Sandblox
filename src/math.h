#ifndef MATH_H
#define MATH_H

float lerp(float a, float b, float t);
float invLerp(float a, float b, float v);
float dotProd2(SDL_FPoint vecA, SDL_FPoint vecB);
float dotProd3(Vector3 vecA, Vector3 vecB);

Vector4 transMatrix(Vector4 vector, mat4 matrix);
float *multMatrix(mat4 matrixA, mat4 matrixB);

#endif