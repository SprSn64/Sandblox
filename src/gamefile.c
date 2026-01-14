#include "gamefile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cjosn/cJSON.h"
#include "instances.h"
#include "entities.h"
#include "renderer.h"
#include "loader.h"

extern GameWorld game;
extern DataObj gameHeader;

extern void playerInit(DataObj* object);
extern void playerUpdate(DataObj* object);
extern void playerDraw(DataObj* object);
extern void blockDraw(DataObj* object);
extern void homerDraw(DataObj* object);

void (*getFunctionByName(const char* name))(DataObj*) {
    if(!name) return NULL;
    if(strcmp(name, "playerInit") == 0) return playerInit;
    if(strcmp(name, "playerUpdate") == 0) return playerUpdate;
    if(strcmp(name, "playerDraw") == 0) return playerDraw;
    if(strcmp(name, "blockDraw") == 0) return blockDraw;
    if(strcmp(name, "homerDraw") == 0) return homerDraw;
    return NULL;
}

DataType* getClassByName(const char* name) {
    if(strcmp(name, "Player") == 0) return &playerClass;
    if(strcmp(name, "Block") == 0) return &blockClass;
    if(strcmp(name, "Mesh") == 0) return &meshClass;
    if(strcmp(name, "beer drinker") == 0) return &fuckingBeerdrinkerClass;
    return NULL;
}

Mesh* createMeshByType(const char* type, cJSON* params) {
    if(!type || !params) return NULL;
    
    if(strcmp(type, "torus") == 0) {
        if(cJSON_GetArraySize(params) >= 4) {
            return genTorusMesh(
                cJSON_GetArrayItem(params, 0)->valuedouble,
                cJSON_GetArrayItem(params, 1)->valuedouble,
                cJSON_GetArrayItem(params, 2)->valueint,
                cJSON_GetArrayItem(params, 3)->valueint
            );
        }
    } else if(strcmp(type, "cylinder") == 0) {
        if(cJSON_GetArraySize(params) >= 4) {
            return genCylinderMesh(
                cJSON_GetArrayItem(params, 0)->valuedouble,
                cJSON_GetArrayItem(params, 1)->valuedouble,
                cJSON_GetArrayItem(params, 2)->valuedouble,
                cJSON_GetArrayItem(params, 3)->valueint
            );
        }
    }
    return NULL;
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
    cJSON* meshType = cJSON_GetObjectItem(obj, "meshType");
    cJSON* meshParams = cJSON_GetObjectItem(obj, "meshParams");
    cJSON* collision = cJSON_GetObjectItem(obj, "collision");
    
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
            cJSON_GetArrayItem(rot, 0)->valuedouble,
            cJSON_GetArrayItem(rot, 1)->valuedouble,
            cJSON_GetArrayItem(rot, 2)->valuedouble
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
    
    printf("Successfully loaded game file\n");
    return 0;
}

DataObj* createPlayerFromJSON() {
    return game.currPlayer;
}