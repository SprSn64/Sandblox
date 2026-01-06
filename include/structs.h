#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL3/SDL.h>

#define RAD2DEG 180 / 3.14159
#define DEG2RAD 3.14159 / 180

#define KEYBINDCOUNT 12
typedef enum gameKeybinds{
	KEYBIND_W, KEYBIND_S, KEYBIND_A, KEYBIND_D,
	KEYBIND_SPACE, KEYBIND_SHIFT,
	KEYBIND_UP, KEYBIND_DOWN, KEYBIND_LEFT, KEYBIND_RIGHT,
	KEYBIND_I, KEYBIND_O,
} gameKeybinds;

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
	Uint8 r, g, b, a;
} CharColour;

typedef struct DataObj DataObj;

typedef struct{
	char *name;
	//Uint16 id;
	Uint32 propType; //the avaliable options and properties and stuff for the object as an enum probably
	
	void (*init)(DataObj*);
	void (*update)(DataObj*);
	// virgin sm64 geo asm vs chad sandblox per-actor draw function
	void (*draw)(DataObj*);
} DataType;

typedef enum objValues{ //array items for the pointers (the void *values)
	OBJVAL_SCRIPT,
	OBJVAL_COLLIDER, OBJVAL_VELOCITY,
	OBJVAL_MESH, OBJVAL_TEXTURE,
	OBJVAL_MAX, //always put in end
} objValues;

typedef struct DataObj{
	Vector3 pos, scale, rot;
	float *transform;
	CharColour colour;
	char *name;
	DataType* classData;

	void *asVoidptr[OBJVAL_MAX];
	Vector3 asVec3[OBJVAL_MAX];
	int asInt[OBJVAL_MAX];
	float asFloat[OBJVAL_MAX];

	struct DataObj* prev;
	struct DataObj* next;
	struct DataObj* parent;
	struct DataObj* child;
	
	/* connect functions and whatnot
	void (*collide)(void);
	void (*destroy)(void);
	*/
} DataObj;

typedef struct{
	Vector3 pos, rot;
	float fov, zoom, focusDist;
	DataObj* focusObj;
	float *transform;
} Camera;

typedef struct{
	DataObj* headObj;
	Camera* currCamera;
	DataObj* currPlayer;
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
	bool debug, pause, studio, online;
	Uint32 playerID;
	GameWorld *gameWorld;
} ClientData;

typedef struct{
	bool down, pressed, released;
	Uint32 code;
} ButtonMap;

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
	Vector3 outNorm;
} CollsionReturn;

#endif