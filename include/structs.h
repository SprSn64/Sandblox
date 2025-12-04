#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL3/SDL.h>

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

typedef struct{
	float x, y, z;
} Vector3;

typedef struct{
	Uint8 r, g, b;
} CharColour;

typedef struct{
	Vector3 pos, rot;
	float fov;
} Camera;

typedef struct{
	Uint8 *name;
	Uint32 propType; //the avaliable options and properties and stuff for the object as an enum probably
	
	void (*init)(void);
	void (*update)(void);
	void (*draw)(void);
} DataType;

typedef struct DataObj{
	Vector3 pos, scale, ori;
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
	bool debug, pause;
} GlobalData;

typedef struct{
	bool down, pressed, released;
	Uint32 scanCode;
} KeyMap;

#endif