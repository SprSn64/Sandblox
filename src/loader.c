#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "loader.h"
#include "opengl.h"

typedef struct {
	int v;
	int vt;
	int vn;
} FaceIndex;

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

	//printf("Pixel format of image %s is 0x%x\n", path, newSurface->format);
	memcpy(newTex->pixels, newSurface->pixels, newSurface->w*newSurface->h * sizeof(Uint32)); 

	SDL_DestroySurface(newSurface);
	return newTex;
}

TextureRef* textureExists(char* path){
	TextureRef *loopItem = headTexture;
	while(loopItem){
		if(loopItem->filePath == path)
			return loopItem;
		loopItem = loopItem->next;
	}
	return NULL;
}

TextureRef* loadTexture(char* path, bool persistent){
	TextureRef* texCheck = textureExists(path);
	if(texCheck) return texCheck;

	TextureRef* texItem = malloc(sizeof(TextureRef));
	if(!texItem)
		return NULL;
	texItem->filePath = strdup(path);
	texItem->prev = NULL; texItem->next = NULL;
	texItem->persistent = persistent;

	SDL_Texture* image = newTexture(path, SDL_SCALEMODE_LINEAR);
	if(image)texItem->image = image;

	Texture* texture = loadRasterTexture(path);
	if(!texture) return texItem;
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

	tex->next->prev = tex->prev;
	tex->prev->next = tex->next;
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
		//freeTexture(currItem); //double free? what?
		currItem = next;
	}
}

Mesh* meshExists(char* path){
	Mesh *loopItem = headMesh;
	while(loopItem){
		if(loopItem->filePath == path)
			return loopItem;
		loopItem = loopItem->next;
	}
	return NULL;
}

static void objCount(const char *path, Uint32 *out_v, Uint32 *out_vt, Uint32 *out_vn, Uint32 *out_faces) {
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[512];
    Uint32 v = 0, vt = 0, vn = 0, faces = 0;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == 'v' && line[1] == ' ')
            v++;
        else if (line[0] == 'v' && line[1] == 't')
            vt++;
        else if (line[0] == 'v' && line[1] == 'n')
            vn++;
        else if (line[0] == 'f' && line[1] == ' ') {
            int verts = 0;
            char *p = line + 2;
            while (*p) {
                while (*p == ' ') p++;
                if (*p == '\0' || *p == '\n') break;
                verts++;
                while (*p && *p != ' ') p++;
            }

            if (verts >= 3)
                faces += verts - 2;
        }
    }
    fclose(f);

    *out_v = v;
    *out_vt = vt;
    *out_vn = vn;
    *out_faces = faces;
}

static FaceIndex parseFaceToken(const char *tok) {
    FaceIndex fi = { -1, -1, -1 };
    fi.v = atoi(tok) - 1;

    const char *s = strchr(tok, '/');
    if (!s) return fi;

    s++;
    if (*s != '/') {
        fi.vt = atoi(s) - 1;
        s = strchr(s, '/');
    }
    if (s && *s == '/') {
        s++;
        fi.vn = atoi(s) - 1;
    }

    return fi;
}

int loadMtlFile(const char *objPath, const char *mtlName, MeshMtlEntry *materials, int maxCount) {
    char mtlPath[512];

    strcpy(mtlPath, objPath);
    char *slash = strrchr(mtlPath, '/');
    if (slash) {
        *(slash + 1) = '\0';
        strcat(mtlPath, mtlName);
    } else {
        strcpy(mtlPath, mtlName);
    }


    FILE *f = fopen(mtlPath, "r");
    if (!f) {
        //printf("dihhhhhhh\n");
        return 0;
    }

    char line[512];
    int count = 0;
    MeshMtlEntry *current = NULL;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "newmtl ", 7) == 0) {
            if (count >= maxCount) break;

            current = &materials[count++];
            sscanf(line, "newmtl %127s", current->name);
            current->tex[0] = '\0';
        }
        else if (strncmp(line, "map_Kd ", 7) == 0 && current) {
            sscanf(line, "map_Kd %255s", current->tex);
            printf("mtl tex %s\n", current->tex);
        }
    }

    fclose(f);
    return count;
}

//todo: some uvs are broken?
Mesh* loadMeshFromObj(char *path, bool persistent) {
    Mesh* checkMesh = meshExists(path);
    if(checkMesh) return checkMesh;

    MeshMtlEntry materials[64];
    int materialCount = 0;
    char currentMaterial[128] = "";

    Uint32 vcount = 0, vtcount = 0, vncount = 0, tricount = 0;
    objCount(path, &vcount, &vtcount, &vncount, &tricount);

    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    Mesh *mesh = calloc(1, sizeof(Mesh)); mesh->prev = NULL; mesh->next = NULL;
    mesh->vertCount = vcount; mesh->faceCount = tricount;
    mesh->verts = calloc(vcount, sizeof(MeshVert));
    mesh->faces = calloc(tricount, sizeof(MeshFace));
    
    mesh->meshType = MESHTYPE_FILE;
    mesh->filePath = malloc(sizeof(char) * (strlen(path) + 1)); sprintf(mesh->filePath, "%s", path);

    SDL_FPoint *uvs = calloc(vtcount, sizeof(SDL_FPoint));
    Vector3 *normals = calloc(vncount, sizeof(Vector3));

    char line[512];
    Uint32 vi = 0, vti = 0, vni = 0, fi = 0;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == 'v' && line[1] == ' ') {
		float r, g, b = 1;
            sscanf(line, "v %f %f %f, %f, %f, %f",
                   &mesh->verts[vi].pos.x,
                   &mesh->verts[vi].pos.y,
                   &mesh->verts[vi].pos.z,
			 &r,
			 &g,
			 &b);
		 mesh->verts[vi].colour = (SDL_FColor){r, g, b, 1.0f};
            vi++;
        }
        else if (line[0] == 'v' && line[1] == 't') {
            sscanf(line, "vt %f %f",
                   &uvs[vti].x,
                   &uvs[vti].y);
		uvs[vti].y = 1 - uvs[vti].y;
            vti++;
        }
        else if (line[0] == 'v' && line[1] == 'n') {
            sscanf(line, "vn %f %f %f",
                   &normals[vni].x,
                   &normals[vni].y,
                   &normals[vni].z);
            vni++;
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            FaceIndex idx[16];
            int count = 0;

            char *tok = strtok(line + 2, " \t\r\n");
            while (tok && count < 16) {
                idx[count++] = parseFaceToken(tok);
                tok = strtok(NULL, " \t\r\n");
            }
            for (int i = 1; i + 1 < count; i++) {
                MeshFace *face = &mesh->faces[fi++];
                MeshVert *a = &mesh->verts[idx[0].v];
                MeshVert *b = &mesh->verts[idx[i].v];
                MeshVert *c = &mesh->verts[idx[i + 1].v];
                if (idx[0].vt >= 0) a->uv = uvs[idx[0].vt];
                if (idx[i].vt >= 0) b->uv = uvs[idx[i].vt];
                if (idx[i+1].vt >= 0) c->uv = uvs[idx[i+1].vt];
                if (idx[0].vn >= 0) a->norm = normals[idx[0].vn];
                if (idx[i].vn >= 0) b->norm = normals[idx[i].vn];
                if (idx[i+1].vn >= 0) c->norm = normals[idx[i+1].vn];
                face->vertA = idx[0].v;
                face->vertB = idx[i].v;
                face->vertC = idx[i + 1].v;

                face->material.tex[0] = '\0';

                for (int mtrl = 0; mtrl < materialCount; mtrl++) {
                    if (strcmp(materials[mtrl].name, currentMaterial) == 0) {
                        strcpy(face->material.tex, materials[mtrl].tex);
                        break;
                    }
                }
            }
        } else if (strncmp(line, "mtllib ", 7) == 0) {
            char mtlName[256];
            sscanf(line, "mtllib %255s", mtlName);
            materialCount = loadMtlFile(path, mtlName, materials, 64);
        } else if (strncmp(line, "usemtl ", 7) == 0) {
            sscanf(line, "usemtl %127s", currentMaterial);
        }
    }

    fclose(f);
    free(uvs);
    free(normals);

    openGlGenBuffers(mesh);
    mesh->persistent = persistent;

    if(!headMesh){
        headMesh = mesh;
        return mesh;
    }

    Mesh *loopItem = headMesh;
    while(loopItem->next){
        loopItem = loopItem->next;
    }
    mesh->prev = loopItem;
    loopItem->next = mesh;

    return mesh;
}

void freeMesh(Mesh* mesh){
	if(!mesh) return;
	free(mesh->verts); free(mesh->faces);
	if(mesh->filePath) free(mesh->filePath);

	mesh->next->prev = mesh->prev;
	mesh->prev->next = mesh->next;
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
		//freeMesh(currItem); //still a double free?
		currItem = next;
	}
}

char* loadTextFile(char* dir){ //code salvaged from first attempt
    FILE *file = fopen(dir, "r");
    if (!file){
        return NULL;
        printf("Couldn't find %s.\n", dir);
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
    buffer[readSize] = '\0'; // null-terminate

    fclose(file);
    return buffer;
}

char* joinDirectories(char* dirA, char* dirB){
    // "home/jerma985/johns/eviljohns" + "../../peters" -> "home/jerma985/peters"
    char *output = malloc(1024); strcpy(output, dirA);

    if(dirB[0] == '.' && dirB[1] == '/')
        sprintf(output, "%s%s", dirA, dirB + 1);

    printf("%s\n", output);
    return output;
}

//Stupid useless code because i was foolish! and fool was i!
extern char* clientPath;
char* formatDirectory(char* dir){
    // "$CLIENT/assets/models/primitives/sphere.obj" -> "home/jerma985/epiccoolgames/Sandblox/assets/models/primitives/sphere.obj"
    char* output = malloc(512 * sizeof(Uint8));

    int slashLoc = strcspn(dir, "/");
    char* stringPiece = malloc(slashLoc * sizeof(Uint8));
    strncpy(stringPiece, dir, slashLoc);

    if(!strcmp(stringPiece, "$CLIENT")){
        sprintf(output, "%s%s", clientPath, dir + slashLoc + 1);
        printf("!-- String %s is in client\n", output);
    }else
        strcpy(output, dir);

    printf("!-- output is %s\n", output);

    free(stringPiece);

    return output;
}