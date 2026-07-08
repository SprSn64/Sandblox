#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mesh.h"
#include "opengl.h"
#include "math.h"

Mesh* headMesh = NULL;

Mesh* meshExists(char* path){
	Mesh *loopItem = headMesh;
	while(loopItem){
        if (loopItem->filePath) {
            if(strcmp(loopItem->filePath, path) == 0)
                return loopItem;
        }
		loopItem = loopItem->next;
	}
	return NULL;
}

Mesh *addMeshToList(Mesh* mesh) {
    if (!headMesh) {
        return headMesh = mesh;
    }

    Mesh *it = headMesh;
    while (it->next) it = it->next;

    it->next = mesh;
    mesh->prev = it;
    return mesh;
}

typedef struct {
	int vert;
	int vertUV;
	int vertNorm;
} FaceIndex;

static void objCount(const char *path, Uint32 *out_v, Uint32 *out_vt, Uint32 *out_vn, Uint32 *out_faces) {
    FILE *file = fopen(path, "r");
    if (!file) return;

    char line[512];
    Uint32 verts = 0, vertUVs = 0, vertNorms = 0, faces = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ')
            verts++;
        else if (line[0] == 'v' && line[1] == 't')
            vertUVs++;
        else if (line[0] == 'v' && line[1] == 'n')
            vertNorms++;
        else if (line[0] == 'f' && line[1] == ' ') {
            int faceVerts = 0;
            char *pointer = line + 2;
            while (*pointer) {
                while (*pointer == ' ') pointer++;
                if (*pointer == '\0' || *pointer == '\n') break;
                faceVerts++;
                while (*pointer && *pointer != ' ') pointer++;
            }

            if (faceVerts >= 3)
                faces += faceVerts - 2;
        }
    }
    fclose(file);

    *out_v = verts;
    *out_vt = vertUVs;
    *out_vn = vertNorms;
    *out_faces = faces;
}

static FaceIndex parseFaceToken(const char *tok) {
	FaceIndex index = {-1, -1, -1};
	index.vert = atoi(tok) - 1;

	const char *firstSlash = strchr(tok, '/');
	if (!firstSlash) return index;

	firstSlash++;
	if (*firstSlash != '/') {
		index.vertUV = atoi(firstSlash) - 1;
		firstSlash = strchr(firstSlash, '/');
	}
	if (firstSlash && *firstSlash == '/') {
		firstSlash++;
		index.vertNorm = atoi(firstSlash) - 1;
	}

	return index;
}

int loadMtlFile(const char *objPath, const char *mtlName, MeshMaterial *materials, int maxCount) {
	char mtlPath[512];

	strcpy(mtlPath, objPath);
	char *lastSlash = strrchr(mtlPath, '/');
	if (lastSlash) {
		*(lastSlash + 1) = '\0';
 		strcat(mtlPath, mtlName);
	} else {
		strcpy(mtlPath, mtlName);
	}

	FILE *file = fopen(mtlPath, "r");
	if (!file) {
		return 0;
	}

	char line[512];
	int count = 0;
	MeshMaterial *current = NULL;

	while (fgets(line, sizeof(line), file)){
		if (strncmp(line, "newmtl ", 7) == 0) {
			if (count >= maxCount) break;

			current = &materials[count++];
			sscanf(line, "newmtl %127s", current->name);
			current->tex[0] = '\0';
		} else if (strncmp(line, "map_Kd ", 7) == 0 && current) {
			sscanf(line, "map_Kd %255s", current->tex);
			printf("mtl tex %s\n", current->tex);

			current->texture = loadTexture(current->tex, false);
		}
	}

	fclose(file);
	return count;
}

Mesh* loadMeshFromObj(char *path, bool persistent) {
    Mesh* checkMesh = meshExists(path);
    if (checkMesh) return checkMesh;

    FILE *file = fopen(path, "r");
    if (!file) return NULL;

    MeshMaterial *materials = calloc(1, sizeof(MeshMaterial) * 64);
    int materialCount = 0;
    char currentMaterial[128] = "";

    Uint32 vcount = 0, vtcount = 0, vncount = 0, tricount = 0;
    objCount(path, &vcount, &vtcount, &vncount, &tricount);

    Mesh *mesh = calloc(1, sizeof(Mesh));

    mesh->faceCount = tricount;
    mesh->faces = calloc(tricount, sizeof(MeshFace));
    mesh->materials = materials;

    mesh->meshType = MESHTYPE_FILE;
    mesh->filePath = strdup(path);

    Vector3 *positions = calloc(vcount, sizeof(Vector3));
    SDL_FPoint *uvs = calloc(vtcount, sizeof(SDL_FPoint));
    Vector3 *normals = calloc(vncount, sizeof(Vector3));

    MeshVert *tempVerts = calloc(1, sizeof(MeshVert) * tricount * 3);
    Uint32 tempVertCount = 0;

    char line[512];
    Uint32 v = 0, vt = 0, vn = 0, f = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            float r = 1, g = 1, b = 1;

            sscanf(line,
                   "v %f %f %f %f %f %f",
                   &positions[v].x,
                   &positions[v].y,
                   &positions[v].z,
                   &r, &g, &b
				);

            v++;
        }
        else if (line[0] == 'v' && line[1] == 't') {
            sscanf(line, "vt %f %f",
                   &uvs[vt].x,
                   &uvs[vt].y
				);

            uvs[vt].y = 1.0f - uvs[vt].y;
            vt++;
        }
        else if (line[0] == 'v' && line[1] == 'n') {
            sscanf(line, "vn %f %f %f",
                   &normals[vn].x,
                   &normals[vn].y,
                   &normals[vn].z
				);

            vn++;
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            FaceIndex indexes[16];
            int count = 0;

            char *tok = strtok(line + 2, " \t\r\n");
            while (tok && count < 16) {
                indexes[count++] = parseFaceToken(tok);
                tok = strtok(NULL, " \t\r\n");
            }

            for (int i = 1; i + 1 < count; i++) {
                MeshFace *face = &mesh->faces[f++];

                FaceIndex ia = indexes[0];
                FaceIndex ib = indexes[i];
                FaceIndex ic = indexes[i + 1];

                // A
                MeshVert *va = &tempVerts[tempVertCount];
                va->pos = positions[ia.vert];
                va->uv  = (ia.vertUV >= 0) ? uvs[ia.vertUV] : (SDL_FPoint){0,0};
                va->norm = (ia.vertNorm >= 0) ? normals[ia.vertNorm] : (Vector3){0,0,0};
                face->vertA = tempVertCount++;

                // B
                MeshVert *vb = &tempVerts[tempVertCount];
                vb->pos = positions[ib.vert];
                vb->uv  = (ib.vertUV >= 0) ? uvs[ib.vertUV] : (SDL_FPoint){0,0};
                vb->norm = (ib.vertNorm >= 0) ? normals[ib.vertNorm] : (Vector3){0,0,0};
                face->vertB = tempVertCount++;

                // C
                MeshVert *vc = &tempVerts[tempVertCount];
                vc->pos = positions[ic.vert];
                vc->uv  = (ic.vertUV >= 0) ? uvs[ic.vertUV] : (SDL_FPoint){0,0};
                vc->norm = (ic.vertNorm >= 0) ? normals[ic.vertNorm] : (Vector3){0,0,0};
                face->vertC = tempVertCount++;

                face->material = NULL;

                for (int m = 0; m < materialCount; m++) {
                    if (strcmp(materials[m].name, currentMaterial) == 0) {
                        face->material = &materials[m];
                        break;
                    }
                }
            }
        } else if (strncmp(line, "mtllib ", 7) == 0) {
            char mtlName[256];
            sscanf(line, "mtllib %255s", mtlName);
            materialCount = loadMtlFile(path, mtlName, materials, 64);
        }
        else if (strncmp(line, "usemtl ", 7) == 0) {
            sscanf(line, "usemtl %127s", currentMaterial);
        }
    }

    fclose(file);

    free(mesh->verts);
    mesh->verts = tempVerts;
    mesh->vertCount = tempVertCount;

    free(positions);
    free(uvs);
    free(normals);

    openGlGenBuffers(mesh);

    mesh->persistent = persistent;

    return addMeshToList(mesh);
}

Mesh* genTorusMesh(float outerRad, float innerRad, Uint16 ringRes, Uint16 ringCount){
	if(ringRes < 3 || ringCount < 3) return NULL;
	Uint32 vertCount = ringRes * ringCount;
	Uint32 faceCount = vertCount * 2;
	
	MeshVert* newVerts = calloc(1, sizeof(MeshVert) * vertCount);
	MeshFace* newFaces = calloc(1, sizeof(MeshFace) * faceCount);
	
	float angleRes = PI * 2 / ringRes;
	float angleRing = PI * 2 / ringCount;
	for(Uint32 i=0; i<vertCount; i++){
		float newAngle = angleRing * floor(i / ringRes);
		newVerts[i].pos = (Vector3){
			(outerRad + SDL_cos(angleRes * (i % ringRes)) * innerRad) * SDL_cos(newAngle), 
			SDL_sin(angleRes * (i % ringRes)) * innerRad, 
			(outerRad + SDL_cos(angleRes * (i % ringRes)) * innerRad) * SDL_sin(newAngle),
		};
		newVerts[i].norm = (Vector3){
			SDL_cos(angleRes * (i % ringRes)) * SDL_cos(newAngle),
			SDL_sin(angleRes * (i % ringRes)),
			SDL_cos(angleRes * (i % ringRes)) * SDL_sin(newAngle),
		};
		newVerts[i].uv = (SDL_FPoint){
			newAngle / (2 * PI),
			angleRes * (i % ringRes) / (2 * PI),
		};
	}
	
	for(Uint32 i=0; i<faceCount/2; i++){
		Uint32 newI = i * 2;
		//MeshVert *quadVerts[4] = {&newVerts[i], &newVerts[(i+1) % vertCount], &newVerts[(i + ringRes) % vertCount], &newVerts[(i+1 + ringRes) % vertCount]};
		
		newFaces[newI] = (MeshFace){i, (i+1) % vertCount, (i + ringRes) % vertCount, 0};
		newFaces[(newI + 1)] = (MeshFace){(i + ringRes) % vertCount, (i+1) % vertCount, (i+1 + ringRes) % vertCount, 0};
	}
	
	Mesh* newMesh = calloc(1, sizeof(Mesh)); newMesh->persistent = false;
	newMesh->vertCount = vertCount; newMesh->verts = newVerts; newMesh->faceCount = faceCount; newMesh->faces = newFaces; 
	newMesh->meshType = MESHTYPE_TORUS;

	return addMeshToList(newMesh);
}

Mesh* genCylinderMesh(float btmRad, float topRad, float length, int res){
	if(fabs(btmRad) + fabs(topRad) == 0 || length == 0 || res < 3) return NULL;
	Uint32 vertCount = res * 2;
	Uint32 faceCount = 4 * res - 4;
	
	MeshVert* newVerts = calloc(1, sizeof(MeshVert) * vertCount);
	MeshFace* newFaces = calloc(1, sizeof(MeshFace) * faceCount);
	
	float angleRes = PI * 2 / res;
	for(Uint32 i=0; i<vertCount; i++){
		float sideRad = lerp(btmRad, topRad, floor(i / res));
		newVerts[i].pos = (Vector3){
			SDL_cos(angleRes * (i % res)) * sideRad,
			(1 - 2 * floor(i / res)) * length / 2,
			SDL_sin(angleRes * (i % res)) * sideRad,
		};
		newVerts[i].norm = normalize3((Vector3){
			SDL_cos(angleRes * (i % res)),
			(1 - floor(i / res) * 2) / 2,
			SDL_sin(angleRes * (i % res)),
		});
	}
	
	for(int i=0; i<res; i++){
		int newI = i * 2;
		//MeshVert *quadVerts[4] = {&newVerts[i], &newVerts[(i+1) % res], &newVerts[(i + res)], &newVerts[(i+1)%res + res]};
		
		newFaces[newI] = (MeshFace){i, (i+1) % res, i + res, 0};
		newFaces[(newI + 1)] = (MeshFace){i + res, (i+1) % res, (i+1)%res + res, 0};
	}
	
	for(int i=0; i<res-2; i++){
		int newI = 2 * res + i;
		newFaces[newI] = (MeshFace){i, 0, (i + 1) % res, 0};
		newFaces[newI + res - 2] = (MeshFace){res, i + res, res + (i+1) % res, 0};
	}
	
	Mesh* newMesh = calloc(1, sizeof(Mesh)); newMesh->persistent = false;
	newMesh->vertCount = vertCount; newMesh->verts = newVerts; newMesh->faceCount = faceCount; newMesh->faces = newFaces; 
	newMesh->meshType = MESHTYPE_CYLINDER;
	return addMeshToList(newMesh);
}

Mesh* genPlaneMesh(float xScale, float yScale, Uint16 xRes, Uint16 yRes){
	if(fabs(xScale) + fabs(yScale) == 0 || xRes + yRes == 0) return 0;
	
	Uint32 vertCount = (xRes + 1) * (yRes + 1);
	Uint32 faceCount = xRes * yRes * 2;
	
	MeshVert* newVerts = calloc(1, sizeof(MeshVert) * vertCount);
	MeshFace* newFaces = calloc(1, sizeof(MeshFace) * faceCount);
	
	for(Uint32 i=0; i<vertCount; i++){
		newVerts[i].pos = (Vector3){
			floor(i / (xRes + 1)) * (xScale / xRes), 
			0, 
			(i % (xRes + 1)) * (yScale / yRes),
		};
		newVerts[i].norm = (Vector3){0, 1, 0};
		newVerts[i].uv = (SDL_FPoint){floor(i / (xRes + 1)) / xRes, (i % (xRes + 1)) / yRes};
	}
	for(Uint32 i=0; i<faceCount/2; i++){
		Uint32 newI = i * 2;
		
		newFaces[newI] = (MeshFace){i, i+1, i + xRes + 1, 0};
		newFaces[(newI + 1)] = (MeshFace){i + xRes + 1, i+1, i + 2 + xRes, 0};
	}
	
	Mesh* newMesh = calloc(1, sizeof(Mesh)); newMesh->persistent = false;
	newMesh->vertCount = vertCount; newMesh->verts = newVerts; newMesh->faceCount = faceCount; newMesh->faces = newFaces; 
	newMesh->meshType = MESHTYPE_PLANE;
	return addMeshToList(newMesh);
}

void freeMesh(Mesh* mesh){
	if(!mesh) return;
	free(mesh->verts); free(mesh->faces);
	if(mesh->filePath) free(mesh->filePath);
	if(mesh->materials) free(mesh->materials);

	if(headMesh == mesh)
		headMesh = mesh->next;

	if(mesh->next)mesh->next->prev = mesh->prev;
	if(mesh->prev)mesh->prev->next = mesh->next;
	free(mesh);
}

void cleanupMeshes(bool soft){
	Mesh* currItem = headMesh;
	while (currItem) {
		if(soft && currItem->persistent){
			currItem = currItem->next;
			continue;
		}
		Mesh *next = currItem->next;
		freeMesh(currItem); 
		currItem = next;
	}
}