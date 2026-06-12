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
	if(texCheck) return texCheck;

	TextureRef* texItem = malloc(sizeof(TextureRef));
	if(!texItem)
		return NULL;
	texItem->filePath = strdup(path);
	texItem->prev = NULL; texItem->next = NULL;
	texItem->persistent = persistent;

	//SDL_Texture* image = newTexture(path, SDL_SCALEMODE_LINEAR);
	//if(image)texItem->image = image;

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
		freeTexture(currItem); //double free? what?
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

	const char *s = strchr(tok, '/');
	if (!s) return index;

	s++;
	if (*s != '/') {
		index.vertUV = atoi(s) - 1;
		s = strchr(s, '/');
	}
	if (s && *s == '/') {
		s++;
		index.vertNorm = atoi(s) - 1;
	}

	return index;
}

int loadMtlFile(const char *objPath, const char *mtlName, MeshMtlEntry *materials, int maxCount) {
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
		//printf("dihhhhhhh\n");
		return 0;
	}

	char line[512];
	int count = 0;
	MeshMtlEntry *current = NULL;

	while (fgets(line, sizeof(line), file)){
		if (strncmp(line, "newmtl ", 7) == 0) { //compares if the first 7 letters are equal
			if (count >= maxCount) break;

			current = &materials[count++];
			sscanf(line, "newmtl %127s", current->name);
			current->tex[0] = '\0';
		} else if (strncmp(line, "map_Kd ", 7) == 0 && current) {
			sscanf(line, "map_Kd %255s", current->tex);
			printf("mtl tex %s\n", current->tex);
		}
	}

	fclose(file);
	return count;
}

/*TODO: 
	some uvs are broken?
	mop up this spilled spaghetti!
*/
Mesh* loadMeshFromObj(char *path, bool persistent) {
	Mesh* checkMesh = meshExists(path);
	if(checkMesh) return checkMesh;

    MeshMtlEntry materials[64];
    int materialCount = 0;
    char currentMaterial[128] = "";

	Uint32 vcount = 0, vtcount = 0, vncount = 0, tricount = 0;
	objCount(path, &vcount, &vtcount, &vncount, &tricount);

	FILE *file = fopen(path, "r");
	if (!file) return NULL;

	Mesh *mesh = calloc(1, sizeof(Mesh)); mesh->prev = NULL; mesh->next = NULL;
	mesh->vertCount = vcount; mesh->faceCount = tricount;
    mesh->verts = calloc(vcount, sizeof(MeshVert));
    mesh->faces = calloc(tricount, sizeof(MeshFace));
    
    mesh->meshType = MESHTYPE_FILE;
    mesh->filePath = malloc(sizeof(char) * (strlen(path) + 1)); sprintf(mesh->filePath, "%s", path);

    SDL_FPoint *uvs = calloc(vtcount, sizeof(SDL_FPoint));
	Vector3 *normals = calloc(vncount, sizeof(Vector3));

	char line[512];
	Uint32 vertItems = 0, vertUVItems = 0, vertNormalItems = 0, faceItems = 0;
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == 'v' && line[1] == ' ') {
			float r, g, b = 1;
            	sscanf(line, "v %f %f %f, %f, %f, %f",
				&mesh->verts[vertItems].pos.x,
				&mesh->verts[vertItems].pos.y,
				&mesh->verts[vertItems].pos.z,
				&r,
				&g,
				&b
			);
			mesh->verts[vertItems].colour = (SDL_FColor){r, g, b, 1.0f};
            	vertItems++;
        	} else if (line[0] == 'v' && line[1] == 't') {
            	sscanf(line, "vt %f %f",
				&uvs[vertUVItems].x,
				&uvs[vertUVItems].y
			);
			uvs[vertUVItems].y = 1 - uvs[vertUVItems].y;
			vertUVItems++;
		} else if (line[0] == 'v' && line[1] == 'n') {
			sscanf(line, "vn %f %f %f",
				&normals[vertNormalItems].x,
				&normals[vertNormalItems].y,
				&normals[vertNormalItems].z
			);
			vertNormalItems++;
		} else if (line[0] == 'f' && line[1] == ' ') {
			FaceIndex indexes[16];
			int count = 0;

			char *tok = strtok(line + 2, " \t\r\n");
			while (tok && count < 16) {
				indexes[count++] = parseFaceToken(tok);
				tok = strtok(NULL, " \t\r\n");
			}
			for (int i = 1; i + 1 < count; i++) {
				MeshFace *face = &mesh->faces[faceItems++];
				MeshVert *a = &mesh->verts[indexes[0].vert];
				MeshVert *b = &mesh->verts[indexes[i].vert];
				MeshVert *c = &mesh->verts[indexes[i + 1].vert];
				if (indexes[0].vertUV >= 0) a->uv = uvs[indexes[0].vertUV];
				if (indexes[i].vertUV >= 0) b->uv = uvs[indexes[i].vertUV];
				if (indexes[i+1].vertUV >= 0) c->uv = uvs[indexes[i+1].vertUV];
				if (indexes[0].vertNorm >= 0) a->norm = normals[indexes[0].vertNorm];
				if (indexes[i].vertNorm >= 0) b->norm = normals[indexes[i].vertNorm];
				if (indexes[i+1].vertNorm >= 0) c->norm = normals[indexes[i+1].vertNorm];
				face->vertA = indexes[0].vert;
				face->vertB = indexes[i].vert;
				face->vertC = indexes[i + 1].vert;

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

	fclose(file);
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
		freeMesh(currItem); //still a double free?
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