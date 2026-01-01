#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL3/SDL.h>

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

typedef float mat4[16];

typedef struct{
	float x, y, z;
} Vector3;

typedef struct{
	float x, y, z, w;
} Vector4;

typedef struct{
	Uint8 r, g, b;
} CharColour;

typedef struct{
	Vector3 pos, rot;
	float fov, zoom;
} Camera;

typedef struct DataObj DataObj;

typedef struct{
	Uint8 *name;
	Uint32 propType; //the avaliable options and properties and stuff for the object as an enum probably
	
	void (*init)(DataObj*);
	void (*update)(DataObj*);
	void (*draw)(DataObj*);
} DataType;

typedef struct DataObj{
	Vector3 pos, scale, rot;
	CharColour colour;
	Uint8 *name;
	DataType* class;
	
	float *values;
	
	struct DataObj* prevItem;
	struct DataObj* nextItem;
	struct DataObj* parent;
	struct DataObj* firstChild;
	
	/* connect functions and whatnot
	void (*collide)(void);
	void (*destroy)(void);
	*/
} DataObj;

typedef struct{
	DataObj* headObj;
	Camera* currCamera;
} GameWorld;

typedef struct{
	Uint8 IPv4[4];
	Uint16 IPv6[8];
} IPAddress;

typedef struct{
	IPAddress serverIP, clientIP;
	float avgPing;
	
	//i dont really know how to do server stuff so
} Server;

typedef struct{
	bool debug, pause, studio;
} ClientData;

typedef struct{
	bool down, pressed, released;
	Uint32 scanCode;
} KeyMap;

//collision slop i think

typedef enum CollisionHullShapes{
	COLLHULL_POINT,
	COLLHULL_SPHERE,
	COLLHULL_CUBE,
	COLLHULL_CYLINDER,
	COLLHULL_FUNCTION,
} CollisionHullShapes;

typedef struct{
	Uint32 shape;
	Vector3 pos, rot, scale;
	
	void (*funkyCollision)(void); //custom collision function for COLLHULL_FUNCTION
} CollisionHull;

typedef struct{
	float depth;
	Vector3 outDir;
} CollsionReturn;

#endif