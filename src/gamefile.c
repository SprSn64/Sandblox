#include "gamefile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cjosn/cJSON.h"
#include "instances.h"
#include "entities.h"
#include "renderer.h"
#include "loader.h"

extern ClientData client;
extern GameWorld game;
extern DataObj gameHeader;

extern void playerInit(DataObj* object);
extern void playerUpdate(DataObj* object);
extern void playerDraw(DataObj* object);
extern void blockDraw(DataObj* object);
extern void homerDraw(DataObj* object);

extern void objSpinFunc(DataObj* object);

void (*getFunctionByName(const char* name))(DataObj*) {
    if(!name) return NULL;
    if(!strcmp(name, "playerInit")) return playerInit;
    if(!strcmp(name, "playerUpdate")) return playerUpdate;
    if(!strcmp(name, "playerDraw")) return playerDraw;
    if(!strcmp(name, "blockDraw")) return blockDraw;
    if(!strcmp(name, "homerDraw")) return homerDraw;
    
    if(!strcmp(name, "objSpinFunc")) return objSpinFunc;
    return NULL;
}

DataType* getClassByName(const char* name) {
    if(!strcmp(name, "Player")) return &playerClass;
    if(!strcmp(name, "Block")) return &blockClass;
    if(!strcmp(name, "Mesh")) return &meshClass;
    if(!strcmp(name, "beer drinker")) return &fuckingBeerdrinkerClass;
    if(!strcmp(name, "Group")) return &groupClass;
    if(!strcmp(name, "Accessory")) return &accessoryClass;
    return NULL;
}

Mesh* createMeshByType(const char* type, cJSON* params) {
	if(!type || !params) return NULL;
	if(cJSON_GetArraySize(params) < 4) return NULL;
	Mesh* newMesh = NULL;
    
	if(!strcmp(type, "torus")){
		newMesh = genTorusMesh(
			cJSON_GetArrayItem(params, 0)->valuedouble,
			cJSON_GetArrayItem(params, 1)->valuedouble,
			cJSON_GetArrayItem(params, 2)->valueint,
			cJSON_GetArrayItem(params, 3)->valueint
           );
	}
	if(!strcmp(type, "cylinder")) {
            newMesh = genCylinderMesh(
			cJSON_GetArrayItem(params, 0)->valuedouble,
			cJSON_GetArrayItem(params, 1)->valuedouble,
			cJSON_GetArrayItem(params, 2)->valuedouble,
			cJSON_GetArrayItem(params, 3)->valueint
            );
	}
	if(!strcmp(type, "plane")) {
            newMesh = genPlaneMesh(
			cJSON_GetArrayItem(params, 0)->valuedouble,
			cJSON_GetArrayItem(params, 1)->valuedouble,
			cJSON_GetArrayItem(params, 2)->valueint,
			cJSON_GetArrayItem(params, 3)->valueint
            );
	}
	return newMesh;
}

DataObj* createObjectFromJSON(cJSON* obj, DataObj* parent) {
    if(!obj) return NULL;
    
    cJSON* name = cJSON_GetObjectItem(obj, "name");
    cJSON* className = cJSON_GetObjectItem(obj, "class");
    cJSON* pos = cJSON_GetObjectItem(obj, "pos");
    cJSON* scale = cJSON_GetObjectItem(obj, "scale");
    cJSON* rot = cJSON_GetObjectItem(obj, "rot");
    cJSON* colour = cJSON_GetObjectItem(obj, "colour");
    cJSON* children = cJSON_GetObjectItem(obj, "children");
    cJSON* meshFile = cJSON_GetObjectItem(obj, "mesh");
    cJSON* meshTexture = cJSON_GetObjectItem(obj, "meshTexture");
    cJSON* meshType = cJSON_GetObjectItem(obj, "meshType");
    cJSON* meshParams = cJSON_GetObjectItem(obj, "meshParams");
    cJSON* collision = cJSON_GetObjectItem(obj, "collision");
    cJSON* scriptFile = cJSON_GetObjectItem(obj, "script");
    
    if(!className || !cJSON_IsString(className)) return NULL;
    
    DataType* objClass = getClassByName(className->valuestring);
    if(!objClass) return NULL;
    
    DataObj* newObj = newObject(parent, objClass);
    if(!newObj) return NULL;
    
    if(name && cJSON_IsString(name)) {
        newObj->name = strdup(name->valuestring);
    }
    
    if(pos && cJSON_IsArray(pos) && cJSON_GetArraySize(pos) >= 3) {
        newObj->pos = (Vector3){
            cJSON_GetArrayItem(pos, 0)->valuedouble,
            cJSON_GetArrayItem(pos, 1)->valuedouble,
            cJSON_GetArrayItem(pos, 2)->valuedouble
        };
    }
    
    if(scale && cJSON_IsArray(scale) && cJSON_GetArraySize(scale) >= 3) {
        newObj->scale = (Vector3){
            cJSON_GetArrayItem(scale, 0)->valuedouble,
            cJSON_GetArrayItem(scale, 1)->valuedouble,
            cJSON_GetArrayItem(scale, 2)->valuedouble
        };
    }
    
    if(rot && cJSON_IsArray(rot) && cJSON_GetArraySize(rot) >= 3) {
        newObj->rot = (Vector3){
            cJSON_GetArrayItem(rot, 0)->valuedouble * DEG2RAD,
            cJSON_GetArrayItem(rot, 1)->valuedouble * DEG2RAD,
            cJSON_GetArrayItem(rot, 2)->valuedouble * DEG2RAD
        };
    }
    
    if(colour && cJSON_IsArray(colour) && cJSON_GetArraySize(colour) >= 4) {
        newObj->colour = (CharColour){
            cJSON_GetArrayItem(colour, 0)->valueint,
            cJSON_GetArrayItem(colour, 1)->valueint,
            cJSON_GetArrayItem(colour, 2)->valueint,
            cJSON_GetArrayItem(colour, 3)->valueint,
            0, COLOURMODE_RGB
        };
    }
    
    Mesh* mesh = NULL;
    if(meshFile && cJSON_IsString(meshFile)) {
        mesh = loadMeshFromObj(meshFile->valuestring);
        if(!mesh) {
            printf("Failed to load mesh from file: %s\n", meshFile->valuestring);
        }
    } else if(meshType && meshParams && cJSON_IsString(meshType)) {
        mesh = createMeshByType(meshType->valuestring, meshParams);
        if(!mesh) {
            printf("Failed to generate procedural mesh: %s\n", meshType->valuestring);
        }
    }
    
    if(mesh) {
        newObj->asVoidptr[OBJVAL_MESH] = mesh;
    }
    
    SDL_Texture* texture = NULL;
    if(meshTexture && cJSON_IsString(meshTexture)) {
        texture = newTexture(meshTexture->valuestring, SDL_SCALEMODE_LINEAR);
        if(!texture)
            printf("Failed to load texture from file: %s\n", meshTexture->valuestring);
    }
    if(texture) {
        newObj->asVoidptr[OBJVAL_TEXTURE] = texture;
    }
    
    if(collision && cJSON_IsObject(collision)) {
        cJSON* enabled = cJSON_GetObjectItem(collision, "enabled");
        cJSON* type = cJSON_GetObjectItem(collision, "type");
        
        if(enabled && cJSON_IsBool(enabled)) {
            newObj->asInt[0] = cJSON_IsTrue(enabled) ? 1 : 0; 
        } else {
            newObj->asInt[0] = 1; 
        }
        
        if(type && cJSON_IsString(type)) {
            if(strcmp(type->valuestring, "block") == 0) {
                newObj->asInt[1] = 1; // collision type: block
            } else if(strcmp(type->valuestring, "trigger") == 0) {
                newObj->asInt[1] = 2; // collision type: trigger
            } else {
                newObj->asInt[1] = 0; // collision type: none
            }
        } else {
            // default collision type based on class
            if(newObj->classData->id == 3) { // blockClass
                newObj->asInt[1] = 1; // block collision
            } else {
                newObj->asInt[1] = 0; // no collision
            }
        }
    } else {
        // default collision settings
        if(newObj->classData->id == 3) { // blockClass gets collision by default
            newObj->asInt[0] = 1; // enabled
            newObj->asInt[1] = 1; // block type
        } else {
            newObj->asInt[0] = 0; // disabled
            newObj->asInt[1] = 0; // no collision
        }
    }
    
    if(scriptFile && cJSON_IsString(scriptFile)) {
	  newObj->asVoidptr[OBJVAL_SCRIPT] = getFunctionByName(scriptFile->valuestring);
    }
    
    if(children && cJSON_IsArray(children)) {
        int childCount = cJSON_GetArraySize(children);
        for(int i = 0; i < childCount; i++) {
            cJSON* child = cJSON_GetArrayItem(children, i);
            createObjectFromJSON(child, newObj);
        }
    }
    
    printf("Loaded object: %s (%s) pos[%.1f,%.1f,%.1f] scale[%.1f,%.1f,%.1f] color[%d,%d,%d,%d]\n", 
           newObj->name, className->valuestring, 
           newObj->pos.x, newObj->pos.y, newObj->pos.z,
           newObj->scale.x, newObj->scale.y, newObj->scale.z,
           newObj->colour.r, newObj->colour.g, newObj->colour.b, newObj->colour.a);
    
    return newObj;
}

int loadGameFile(const char* filename) {
    printf("Loading game file: %s\n", filename);
    
    FILE* file = fopen(filename, "r");
    if(!file) {
        printf("Failed to open game file: %s\n", filename);
        return -1;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(fileSize + 1);
    fread(content, 1, fileSize, file);
    content[fileSize] = '\0';
    fclose(file);
    
    cJSON* json = cJSON_Parse(content);
    if(!json) {
        printf("Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
        free(content);
        return -1;
    }
    
    cJSON* objects = cJSON_GetObjectItem(json, "objects");
    if(!objects || !cJSON_IsArray(objects)) {
        printf("No objects array found in JSON\n");
        cJSON_Delete(json);
        free(content);
        return -1;
    }
    
    printf("Found %d objects in JSON\n", cJSON_GetArraySize(objects));
    
    client.pause = true;
    int objectCount = cJSON_GetArraySize(objects);
    for(int i = 0; i < objectCount; i++) {
        cJSON* obj = cJSON_GetArrayItem(objects, i);
        if(obj) {
            DataObj* newObj = createObjectFromJSON(obj, NULL);
            if(newObj) {
                if(newObj->classData == &playerClass) {
                    game.currPlayer = newObj;
                    printf("Set current player to: %s\n", newObj->name ? newObj->name : "unnamed");
                }
            }
        }
    }
    
    cJSON_Delete(json);
    free(content);
    client.pause = false;
    printf("Successfully loaded game file\n");
    return 0;
}

DataObj* createPlayerFromJSON() {
    return game.currPlayer;
}

int saveGameFile(const char* filename) {
    printf("Saving game file: %s\n", filename);
    
    /*FILE* file = fopen(filename, "r");
    if(!file) {
        printf("Failed to open game file: %s\n", filename);
        return -1;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(fileSize + 1);
    fread(content, 1, fileSize, file);
    content[fileSize] = '\0';
    fclose(file);
    
    cJSON* json = cJSON_Parse(content);
    if(!json) {
        printf("Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
        free(content);
        return -1;
    }
    
    cJSON* objects = cJSON_GetObjectItem(json, "objects");
    if(!objects || !cJSON_IsArray(objects)) {
        printf("No objects array found in JSON\n");
        cJSON_Delete(json);
        free(content);
        return -1;
    }
    
    printf("Found %d objects in JSON\n", cJSON_GetArraySize(objects));
    
    client.pause = true;
    int objectCount = cJSON_GetArraySize(objects);
    for(int i = 0; i < objectCount; i++) {
        cJSON* obj = cJSON_GetArrayItem(objects, i);
        if(obj) {
            DataObj* newObj = createObjectFromJSON(obj, NULL);
            if(newObj) {
                if(newObj->classData == &playerClass) {
                    game.currPlayer = newObj;
                    printf("Set current player to: %s\n", newObj->name ? newObj->name : "unnamed");
                }
            }
        }
    }
    
    cJSON_Delete(json);
    free(content);
    client.pause = false;
    printf("Successfully loaded game file\n");*/
    return 0;
}