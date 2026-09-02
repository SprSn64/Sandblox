#ifndef ENTITIES_H
#define ENTITIES_H

#include "structs.h"
#include "renderer.h"

typedef struct PlayerData{
	Uint16 playerID;
	bool movement;

	float moveSpeed;
	float jumpStrength;

	float coyote; //Coyote time
	float coyoteMax;
} PlayerData;
extern DataType playerClass;

typedef struct ParticleEmitter ParticleEmitter;
typedef struct Particle{
	ParticleEmitter* parent;
	TextureRef* texture;
	CharColour colour;
	Vector3 pos; Vector3 vel;
	float size;

	float life;

	struct Particle* prev;
	struct Particle* next;
} Particle;
typedef struct ParticleEmitter{
	Particle* headParticle;
	Vector3 initVel; Vector3 accel;
	float velRand;

	float waitTime; //time between making new particles
	float timer;
} ParticleEmitter;
extern DataType particleClass;

/*void playerInit(DataObj* object);
void playerUpdate(DataObj* object);
void playerDraw(DataObj* object);
void blockDraw(DataObj* object);
void homerDraw(DataObj* object);

void scriptUpdate(dataObj* object);*/

extern DataType blockClass;
extern DataType meshClass;
extern DataType groupClass;
extern DataType cameraClass;
//extern DataType lightClass;
extern DataType imageClass;
extern DataType scriptClass;
extern DataType accessoryClass;
extern DataType armatureClass;

extern DataType fuckingBeerdrinkerClass;

void objSpinFunc(DataObj* object);
void killBrickFunc(DataObj* object);

#endif