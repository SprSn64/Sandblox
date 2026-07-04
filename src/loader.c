#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "loader.h"
#include "opengl.h"

TextureRef* headTexture = NULL;
Mesh* headMesh = NULL;

extern SDL_Renderer* renderer;
SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode){
	SDL_Texture *texture = IMG_LoadTexture(renderer, path);
	if(texture == NULL){
		printf("Issue with loading SDL texture %s!\n", path);
		return NULL;
	}
	SDL_SetTextureScaleMode(texture, scaleMode);
	return texture;
}

Texture* newRasterTexture(Uint16 width, Uint16 height){
	Texture* newTexture = malloc(sizeof(Texture));
	if(!newTexture){
		printf("Failed to generate texture of size %dx%d\n", width, height);
		return NULL;
	}
	newTexture->width = width; newTexture->height = height;
	newTexture->pixels = malloc(width * height * sizeof(Uint32));
	
	if(!newTexture->pixels){
	    free(newTexture);
	    return NULL;
	}
	
	printf("Succesfully made texture of size %dx%d\n", width, height);
	return newTexture;
}

bool freeRasterTexture(Texture* tex){
	if(!tex) return 1;
	free(tex->pixels); free(tex);
	return 0;
}

Texture* loadRasterTexture(char* path){
	SDL_Surface* newSurface = IMG_Load(path); if(!newSurface) return NULL;
	Texture* newTex = newRasterTexture(newSurface->w, newSurface->h);

	memcpy(newTex->pixels, newSurface->pixels, (Uint32)newSurface->w*newSurface->h * sizeof(Uint32)); 

	SDL_DestroySurface(newSurface);
	return newTex;
}

TextureRef* textureExists(char* path){
	TextureRef *loopItem = headTexture;
	while(loopItem){
		if(!strcmp(loopItem->filePath, path))
			return loopItem;
		loopItem = loopItem->next;
	}
	return NULL;
}

TextureRef* loadTexture(char* path, bool persistent){
	TextureRef* texCheck = textureExists(path);
	if(texCheck){
		printf("Texture %s already exists...\n", path);
		return texCheck;
	}

	TextureRef* texItem = calloc(1, sizeof(TextureRef));
	if(!texItem) return NULL;
	texItem->filePath = strdup(path);
	texItem->persistent = persistent;

	Texture* texture = loadRasterTexture(path);
	if(!texture) {
	    free(texItem->filePath);
	    free(texItem);
	    return NULL;
	}
	texItem->texture = texture;

	glGenTextures(1, &texItem->glLoc);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texItem->glLoc);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	if(!headTexture){
		headTexture = texItem;
		return texItem;
	}

	TextureRef *loopItem = headTexture;
	while(loopItem->next){
		loopItem = loopItem->next;
	}
	texItem->prev = loopItem;
	loopItem->next = texItem;

	return texItem;
}

void updateGlTexture(TextureRef* tex){
	glBindTexture(GL_TEXTURE_2D, tex->glLoc);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->texture->width, tex->texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->texture->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void freeTexture(TextureRef* tex){
	if(!tex) return;
	if(tex->image)SDL_DestroyTexture(tex->image);
	if(tex->texture)freeRasterTexture(tex->texture);
	if(tex->glLoc)glDeleteTextures(1, &tex->glLoc);
	if(tex->filePath)free(tex->filePath);

	if(headTexture == tex)
		headTexture = tex->next;

	if(tex->next)tex->next->prev = tex->prev;
	if(tex->prev)tex->prev->next = tex->next;
	free(tex);
}

void cleanupTextures(bool soft){
	TextureRef* currItem = headTexture;
	while (currItem) {
		if(soft && currItem->persistent){
			currItem = currItem->next;
			continue;
		}

		TextureRef *next = currItem->next;
		freeTexture(currItem); 
		currItem = next;
	}
}

Mesh* meshExists(char* path){
	Mesh *loopItem = headMesh;
	while(loopItem){
		if(strcmp(loopItem->filePath, path) == 0)
			return loopItem;
		loopItem = loopItem->next;
	}
	return NULL;
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

    MeshMaterial *materials = malloc(sizeof(MeshMaterial) * 64);
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

    MeshVert *tempVerts = malloc(sizeof(MeshVert) * tricount * 3);
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

    if (!headMesh) {
        headMesh = mesh;
        return mesh;
    }

    Mesh *it = headMesh;
    while (it->next) it = it->next;

    it->next = mesh;
    mesh->prev = it;

    return mesh;
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

char* loadTextFile(char* dir){ 
    FILE *file = fopen(dir, "r");
    if (!file){
        printf("Couldn't find %s.\n", dir);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = malloc(size + 1);
    if (!buffer){
        fclose(file);
        return NULL;
    }

    size_t readSize = fread(buffer, 1, size, file);
    buffer[readSize] = '\0';

    fclose(file);
    return buffer;
}

char* joinDirectories(char* dirA, char* dirB){
    char *output = malloc(1024); strcpy(output, dirA);

    if(dirB[0] == '.' && dirB[1] == '/')
        sprintf(output, "%s%s", dirA, dirB + 1);

    printf("%s\n", output);
    return output;
}

extern char* clientPath;
char* formatDirectory(char* dir){
    char* output = malloc(512 * sizeof(Uint8));

    int slashLoc = strcspn(dir, "/");
    char* stringPiece = malloc((slashLoc + 1) * sizeof(Uint8));
    strncpy(stringPiece, dir, slashLoc);
    stringPiece[slashLoc] = '\0';

    if(!strcmp(stringPiece, "$CLIENT")){
        sprintf(output, "%s%s", clientPath, dir + slashLoc + 1);
        printf("!-- String %s is in client\n", output);
    }else
        strcpy(output, dir);

    printf("!-- output is %s\n", output);

    free(stringPiece);

    return output;
}