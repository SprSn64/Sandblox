#ifndef MATH_H
#define MATH_H

float lerp(float a, float b, float t);
float invLerp(float a, float b, float v);
float dotProd2(SDL_FPoint vecA, SDL_FPoint vecB);
float dotProd3(Vector3 vecA, Vector3 vecB);
float closest(float input, float snap);
bool between(float input, float min, float max);
SDL_FPoint normalize2(SDL_FPoint vec);
Vector3 normalize3(Vector3 vec);
Vector3 reflect(Vector3 incident, Vector3 normal);
Vector3 rotToNorm3(Vector3 rot);

float *newMatrix();
Vector4 matrixMult(Vector4 vector, mat4 matrix);
float *multMatrix(mat4 matrixA, mat4 matrixB);

float *translateMatrix(mat4 matrix, Vector3 move);
float *scaleMatrix(mat4 matrix, Vector3 scale);
float *axisRotMatrix(Uint8 axis, float angle);
float *rotateMatrix(mat4 matrix, Vector3 angle);

float *genMatrix(Vector3 pos, Vector3 scale, Vector3 rot);

//versions of above functions but without generating new ones
void translateMatrix2(mat4 matrix, Vector3 move);
void scaleMatrix2(mat4 matrix, Vector3 scale);

float *perspMatrix(float fov, float aspect, float zNear, float zFar);

#endif