//load txt and obj/custom binary mesh files functions

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "loader.h"

void objCountVT(const char* path, Uint32*amount_V, Uint32*amount_T) {
	Uint32 vcount = 0;
	Uint32 tcount = 0;

	FILE *f = fopen(path, "r");
	if (!f) {
		printf("Couldn't load object file %s\n", path);
		return;
	}

	char line[512];
	while (fgets(line, sizeof(line), f)) {
		if (line[0] == 'v' && line[1] == ' ') {
			vcount++;
		} else if (line[0] == 'f' && line[1] == ' ') {
			tcount++;
		}
	}
	fclose(f);
	
	*amount_V = vcount;
	*amount_T = tcount;
}

Mesh *loadMeshFromObj(const char* path) {
	Uint32 vcount = 0;
	Uint32 tcount = 0;
	objCountVT(path, &vcount, &tcount);

	FILE *f = fopen(path, "r");
	if (!f) {
		printf("Couldn't load object file %s\n", path);
		return NULL;
	}

	Mesh *outMesh = calloc(1, sizeof(Mesh));

	outMesh->vertCount = vcount;
	outMesh->faceCount = tcount;
	outMesh->verts = calloc(vcount, sizeof(MeshVert));
	outMesh->faces = calloc(tcount, sizeof(MeshFace));

	printf("Loading mesh %s, vcount: %u, tcount: %u\n", path, vcount, tcount);

	char line[512];
	Uint32 vi = 0;
	Uint32 ti = 0;
	while (fgets(line, sizeof(line), f)) {
		if (line[0] == 'v' && line[1] == ' ') {
			float x,y,z;
			sscanf(line, "v %f %f %f", &x, &y, &z);
			outMesh->verts[vi].pos.x = x;
			outMesh->verts[vi].pos.y = y;
			outMesh->verts[vi].pos.z = z;
			//printf("VTX %f, %f, %f\n", x,y,z);
			vi++;
		} else if (line[0] == 'f' && line[1] == ' ') {
			int a,b,c;
			sscanf(line, "f %d %d %d", &a, &b, &c);
			a--;b--;c--;
			outMesh->faces[ti].vertA = &outMesh->verts[a];
			outMesh->faces[ti].vertB = &outMesh->verts[b];
			outMesh->faces[ti].vertC = &outMesh->verts[c];
			//printf("FACE %d, %d, %d\n", a,b,c);
			ti++;
		}
	}
	fclose(f);
	return outMesh;
}