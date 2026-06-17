#include "gamefile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cjosn/cJSON.h"
#include "instances.h"
#include "entities.h"
#include "renderer.h"
#include "loader.h"
#include "math.h"

extern ClientData client;
extern DataObj gameHeader;

extern Vector3 lightNormal;
extern SDL_FColor lightColour;
extern SDL_FColor lightAmbient;

DataObj* loadedPlayer = NULL;
extern char* basePath;

extern void objSpinFunc(DataObj* object);

char* collTypes[6] = {"point", "sphere", "block", "cylinder", "function", "trigger"};

Uint32 stringIdInList(const char* string, char** list, Uint32 listLength){
    for(Uint32 i=0; i<listLength; i++){
        if(!strcmp(list[i], string)) return i;
    }
    return listLength + 1;
}

#include "tempMaps/mapInit.h"

void (*getFunctionByName(const char* name))(DataObj*) {
    if(!name) return NULL;
    
    if(!strcmp(name, "objSpinFunc")) return objSpinFunc;
    if(!strcmp(name, "killBrickFunc")) return killBrickFunc;
    if(!strcmp(name, "soulControlFunc")) return soulControlFunc;
    if(!strcmp(name, "knightHoverFunc")) return knightHoverFunc;
    return NULL;
}

DataType* getClassByName(const char* name) {
    if(!strcmp(name, "Player")) return &playerClass;
    if(!strcmp(name, "Block")) return &blockClass;
    if(!strcmp(name, "Mesh")) return &meshClass;
    if(!strcmp(name, "beer drinker")) return &fuckingBeerdrinkerClass;
    if(!strcmp(name, "Camera")) return &cameraClass;
    if(!strcmp(name, "Group")) return &groupClass;
    if(!strcmp(name, "Image")) return &imageClass;
    if(!strcmp(name, "Script")) return &scriptClass;
    if(!strcmp(name, "Accessory")) return &accessoryClass;
    if(!strcmp(name, "Armature")) return &armatureClass;
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

char currPath[1024];
DataObj* createObjectFromJSON(cJSON* obj, DataObj* parent) {
    if(!obj) return NULL;
    
    cJSON* name = cJSON_GetObjectItem(obj, "name");
    cJSON* className = cJSON_GetObjectItem(obj, "class");
    cJSON* pos = cJSON_GetObjectItem(obj, "pos");
    cJSON* scale = cJSON_GetObjectItem(obj, "scale");
    cJSON* rot = cJSON_GetObjectItem(obj, "rot");
    cJSON* colour = cJSON_GetObjectItem(obj, "colour");
    cJSON* children = cJSON_GetObjectItem(obj, "children");
    cJSON* texture = cJSON_GetObjectItem(obj, "texture");
    cJSON* meshFile = cJSON_GetObjectItem(obj, "mesh");
    cJSON* meshType = cJSON_GetObjectItem(obj, "meshType");
    cJSON* meshParams = cJSON_GetObjectItem(obj, "meshParams");
    cJSON* collision = cJSON_GetObjectItem(obj, "collision");
    cJSON* scriptFile = cJSON_GetObjectItem(obj, "script");
    cJSON* isPlayer = cJSON_GetObjectItem(obj, "isPlayer");
    
    if(!className || !cJSON_IsString(className)) return NULL;
    
    DataType* objClass = getClassByName(className->valuestring);
    if(!objClass) return NULL;
    
    DataObj* newParent = parent;
    if(!parent) newParent = client.gameWorld->headObj;
    DataObj* newObj = newObject(objClass);
    if(!newObj) return NULL;
    parentObject(newObj, newParent);
    
    if(isPlayer && cJSON_IsBool(isPlayer) && cJSON_IsTrue(isPlayer) && !loadedPlayer)
	    loadedPlayer = newObj;
    
    if(name && cJSON_IsString(name))
        newObj->name = strdup(name->valuestring);
    
    if(pos && cJSON_IsArray(pos) && cJSON_GetArraySize(pos) >= 3)
        newObj->pos = (Vector3){
            cJSON_GetArrayItem(pos, 0)->valuedouble,
            cJSON_GetArrayItem(pos, 1)->valuedouble,
            cJSON_GetArrayItem(pos, 2)->valuedouble
        };
    
    if(scale && cJSON_IsArray(scale) && cJSON_GetArraySize(scale) >= 3)
        newObj->scale = (Vector3){
            cJSON_GetArrayItem(scale, 0)->valuedouble,
            cJSON_GetArrayItem(scale, 1)->valuedouble,
            cJSON_GetArrayItem(scale, 2)->valuedouble
        };
    
    if(rot && cJSON_IsArray(rot) && cJSON_GetArraySize(rot) >= 3)
        newObj->rot = (Vector3){
            cJSON_GetArrayItem(rot, 0)->valuedouble * DEG2RAD,
            cJSON_GetArrayItem(rot, 1)->valuedouble * DEG2RAD,
            cJSON_GetArrayItem(rot, 2)->valuedouble * DEG2RAD
        };
    
    if(colour && cJSON_IsArray(colour) && cJSON_GetArraySize(colour) >= 4)
        newObj->colour = (CharColour){
            cJSON_GetArrayItem(colour, 0)->valueint,
            cJSON_GetArrayItem(colour, 1)->valueint,
            cJSON_GetArrayItem(colour, 2)->valueint,
            cJSON_GetArrayItem(colour, 3)->valueint,
            0, COLOURMODE_RGB
        };
    
    Mesh* mesh = NULL;
    if(meshFile && cJSON_IsString(meshFile)) {
        if(meshFile->valuestring[0] == '.' && meshFile->valuestring[1] == '/'){
            char* meshString = joinDirectories(currPath, meshFile->valuestring);
            mesh = loadMeshFromObj(meshString, false);
            free(meshString);
            sprintf(mesh->filePath, "%s", meshFile->valuestring);
        }else
            mesh = loadMeshFromObj(meshFile->valuestring, false);
        if(!mesh){
            char* warningLog = malloc(512);
		sprintf(warningLog, "Failed to load mesh from %s\n", meshFile->valuestring);
		logToConsole(warningLog, CONSOLELOG_WARNING);
        }
    } else if(meshType && meshParams && cJSON_IsString(meshType)) {
        mesh = createMeshByType(meshType->valuestring, meshParams);
        if(!mesh)
            printf("Failed to generate procedural mesh: %s\n", meshType->valuestring);
    }
    
    if(mesh)
        newObj->props[OBJVAL_MESH] = mesh;
    
    TextureRef* tex = NULL;
    if(texture && cJSON_IsString(texture)) {
        if(texture->valuestring[0] == '.' && texture->valuestring[1] == '/'){
            char* texString = joinDirectories(currPath, texture->valuestring);
            tex = loadTexture(texString, false);
            free(texString);
        }else 
            tex = loadTexture(texture->valuestring, false);
	  
        if(!tex){
            char* warningLog = malloc(512);
		sprintf(warningLog, "Failed to load texture from %s\n", texture->valuestring);
		logToConsole(warningLog, CONSOLELOG_WARNING);
        }
    }
    if(tex) {
        	newObj->props[OBJVAL_TEXTURE] = tex;
    }
    
    //CollisionHull *block->props[OBJVAL_COLLIDER]
    if(collision && cJSON_IsObject(collision)) {
        cJSON* enabled = cJSON_GetObjectItem(collision, "enabled");
        cJSON* type = cJSON_GetObjectItem(collision, "type");
	  
	    CollisionHull *collider = malloc(sizeof(CollisionHull));
        
        if(!(enabled && cJSON_IsBool(enabled))) {
		  free(collider); goto colliderLoadSkip;
        }
        
        if(type && cJSON_IsString(type)) {
            Uint32 collShape = stringIdInList(type->valuestring, collTypes, 6);
            if(!collShape)
                free(collider);
            else collider->shape = collShape;
        } else {
            // default collision type based on class
            if(newObj->classData->id == blockClass.id) // blockClass
                collider->shape = COLLHULL_CUBE; // block collision
            else
                free(collider); // no collision
        }
	    colliderLoadSkip:
	    newObj->props[OBJVAL_COLLIDER] = collider;
    }
    
    if(scriptFile && cJSON_IsString(scriptFile)) {
	    ScriptItem *newScript = malloc(sizeof(ScriptItem));

        newScript->func = getFunctionByName(scriptFile->valuestring); newScript->funcName = strdup(scriptFile->valuestring);  
        if(newScript->func != NULL)
            newObj->props[OBJVAL_SCRIPT] = newScript;
        else
            free(newScript);
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

extern bool playerEnabled;
int loadGameFile(const char* filename) {
    printf("Loading game file: %s...\n", filename);
    
    FILE* file = fopen(filename, "r");
    if(!file) {
        printf("Failed to open game file: %s\n", filename);
        return -1;
    }
    strcpy(currPath, filename);
    char* dirSlash = strrchr(currPath, '/'); 
    *dirSlash = '\0';
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(fileSize + 1);
    int readThing = fread(content, 1, fileSize, file); (void)readThing;
    content[fileSize] = '\0';
    fclose(file);
    
    cJSON* json = cJSON_Parse(content);
    if(!json) {
        printf("Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
        free(content);
        return -1;
    }

    cJSON* version = cJSON_GetObjectItem(json, "version");
    if(version && cJSON_IsString(version)){
        if(strcmp(version->valuestring, client.version) > 0){
        	sendPopup("Map file is from a future version!", NULL, NULL, 5);
        	return -1;
        }
    }

    cJSON* objects = cJSON_GetObjectItem(json, "objects");
    if(!objects || !cJSON_IsArray(objects)) {
        printf("No objects array found in JSON\n");
        cJSON_Delete(json);
        free(content);
        return -1;
    }
    
    printf("Found %d objects in JSON\n", cJSON_GetArraySize(objects));
    loadedPlayer = NULL;
    
    client.pause = true;
    cleanupTextures(true); cleanupMeshes(true); clearConsole();
    lesserCleanupObjects(client.gameWorld->headObj);

    cJSON* plrEnabled = cJSON_GetObjectItem(json, "playerEnabled");
    if(plrEnabled && cJSON_IsBool(plrEnabled))
		playerEnabled = cJSON_IsTrue(plrEnabled);
    else
       	playerEnabled = true;
    
    cJSON* lightDir = cJSON_GetObjectItem(json, "lightDir");
    if(lightDir && cJSON_IsArray(lightDir) && cJSON_GetArraySize(lightDir) >= 3)
        lightNormal = rotToNorm3((Vector3){
            cJSON_GetArrayItem(lightDir, 0)->valuedouble * DEG2RAD,
            cJSON_GetArrayItem(lightDir, 1)->valuedouble * DEG2RAD,
            cJSON_GetArrayItem(lightDir, 2)->valuedouble * DEG2RAD
        });

    cJSON* lightCol = cJSON_GetObjectItem(json, "lightColour");
    if(lightCol && cJSON_IsArray(lightCol) && cJSON_GetArraySize(lightCol) >= 4){
        lightColour = (SDL_FColor){
            (float)(cJSON_GetArrayItem(lightCol, 0)->valueint) / 255,
            (float)(cJSON_GetArrayItem(lightCol, 1)->valueint) / 255,
            (float)(cJSON_GetArrayItem(lightCol, 2)->valueint) / 255,
            (float)(cJSON_GetArrayItem(lightCol, 3)->valueint) / 255
        };
    }else
        lightColour = (SDL_FColor){1, 1, 1, 1};

    cJSON* lightAmb = cJSON_GetObjectItem(json, "lightAmb");
    if(lightAmb && cJSON_IsArray(lightAmb) && cJSON_GetArraySize(lightAmb) >= 4){
        lightAmbient = (SDL_FColor){
            (float)(cJSON_GetArrayItem(lightAmb, 0)->valueint) / 255,
            (float)(cJSON_GetArrayItem(lightAmb, 1)->valueint) / 255,
            (float)(cJSON_GetArrayItem(lightAmb, 2)->valueint) / 255,
            (float)(cJSON_GetArrayItem(lightAmb, 3)->valueint) / 255
        };
    }else
        lightAmbient = (SDL_FColor){0.25, 0.25, 0.3, 1};

    cJSON* skybox = cJSON_GetObjectItem(json, "skybox");
    TextureRef* skyboxTex = NULL;
    if(skybox && cJSON_IsString(skybox)) {
        if(skybox->valuestring[0] == '.' && skybox->valuestring[1] == '/'){
            char* texString = joinDirectories(currPath, skybox->valuestring);
            skyboxTex = loadTexture(texString, false);
            free(texString);
        }else 
            skyboxTex = loadTexture(skybox->valuestring, false);
	  
        if(!skyboxTex){
            char* warningLog = malloc(256);
		sprintf(warningLog, "Failed to load texture from file: %s\n", skybox->valuestring);
		logToConsole(warningLog, CONSOLELOG_WARNING);
        }
    }
    client.gameWorld->skybox = skyboxTex;
    
    int objectCount = cJSON_GetArraySize(objects);
    for(int i = 0; i < objectCount; i++) {
        cJSON* obj = cJSON_GetArrayItem(objects, i);
        if(obj)
            /*DataObj* newObj = */createObjectFromJSON(obj, NULL);
    }
	
    client.gameWorld->playerRespawn = 10;
    if(loadedPlayer){
        client.gameWorld->currPlayer = loadedPlayer;
        client.gameWorld->playerRespawn = 0;
        printf("Set current player to: %s\n", loadedPlayer->name ? loadedPlayer->name : "unnamed");
        loadedPlayer = NULL;
    }else
    	client.gameWorld->currPlayer = NULL;
    
    cJSON_Delete(json);
    free(content);
    client.pause = false;
    printf("Successfully loaded gamefile %s\n", filename);
    return 0;
}

/*
cJSON* cJSON_AddNullToObject(cJSON * const object, const char * const name);
cJSON* cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean);
cJSON* cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number);
cJSON* cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);
cJSON* cJSON_AddRawToObject(cJSON * const object, const char * const name, const char * const raw);
cJSON* cJSON_AddObjectToObject(cJSON * const object, const char * const name);
cJSON* cJSON_AddArrayToObject(cJSON * const object, const char * const name);
*/

void addObjToJsonArray(cJSON* array, DataObj* item){
	cJSON* newObj = array;
	cJSON* newArray = array;
	DataObj* child = item->child;
	if(item == client.gameWorld->headObj) goto headerSkip;

	if(item == client.gameWorld->currPlayer) return;

	newObj = cJSON_CreateObject();
	cJSON_AddStringToObject(newObj, "name", item->name);
	cJSON_AddStringToObject(newObj, "class", item->classData->name);
	
	char rawPosition[128];
	sprintf(rawPosition, "[%f, %f, %f]", item->pos.x, item->pos.y, item->pos.z);
	cJSON_AddRawToObject(newObj, "pos", rawPosition);
	
	char rawScale[128];
	sprintf(rawScale, "[%f, %f, %f]", item->scale.x, item->scale.y, item->scale.z);
	cJSON_AddRawToObject(newObj, "scale", rawScale);
	
	char rawRotation[128];
	sprintf(rawRotation, "[%f, %f, %f]", item->rot.x * RAD2DEG, item->rot.y * RAD2DEG, item->rot.z * RAD2DEG);
	cJSON_AddRawToObject(newObj, "rot", rawRotation);
	
	char rawColour[24];
	sprintf(rawColour, "[%d, %d, %d, %d]", item->colour.r, item->colour.g, item->colour.b, item->colour.a);
	cJSON_AddRawToObject(newObj, "colour", rawColour);
	
	Mesh* itemMesh = item->props[OBJVAL_MESH];
	if(itemMesh && itemMesh->meshType == MESHTYPE_FILE)
		cJSON_AddStringToObject(newObj, "mesh", itemMesh->filePath);
	
	TextureRef* itemTex = item->props[OBJVAL_TEXTURE];
	if(itemTex)
		cJSON_AddStringToObject(newObj, "texture", itemTex->filePath);
	
	ScriptItem *itemScript = item->props[OBJVAL_SCRIPT];
	if(itemScript && itemScript->funcName)
		cJSON_AddStringToObject(newObj, "script", itemScript->funcName);
	
	CollisionHull *collider = item->props[OBJVAL_COLLIDER];
	
	if(!collider) goto colliderSkip;
	
	cJSON* collObject = cJSON_AddObjectToObject(newObj, "collision");
	cJSON_AddBoolToObject(collObject, "enabled", collider != NULL);
	cJSON_AddStringToObject(collObject, "type", collTypes[collider->shape]);
	
	colliderSkip:
	
	cJSON_AddItemToArray(array, newObj);
	if(!child) return;
	newArray = cJSON_AddArrayToObject(newObj, "children");
	
	headerSkip:
	
	while (child) {
		DataObj *next = child->next;
		addObjToJsonArray(newArray, child);
		child = next;
	}
}

int saveGameFile(const char* filename){
	printf("Saving game file: %s...\n", filename);
	
	FILE* file = fopen(filename, "w");
	
	cJSON* jsonHeader = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonHeader, "version", client.version);

	if(client.gameWorld->skybox){
		cJSON_AddStringToObject(jsonHeader, "skybox", client.gameWorld->skybox->filePath);
	}

	cJSON* objectArray = cJSON_AddArrayToObject(jsonHeader, "objects");
	
	addObjToJsonArray(objectArray, client.gameWorld->headObj);
	
	fprintf(file, "%s", cJSON_Print(jsonHeader));
	fclose(file);
    
	/*
	char* cJSON_Print(const cJSON *item); (output as readable JSON format string)
	bool cJSON_AddItemToArray(cJSON *array, cJSON *item);
	
	cJSON* cJSON_CreateArray(void);
	cJSON* cJSON_CreateObject(void);
	*/
	printf("Successfully saved gamefile %s\n", filename);
	
	return 0;
}

DataObj* loadPlayerAvatar(){
	DataObj* newPlayer = newObject(&playerClass);
	parentObject(newPlayer, client.gameWorld->headObj);
	client.gameWorld->playerRespawn = 0;

	DataObj* newArmature = newObject(&armatureClass);
	parentObject(newArmature, newPlayer);

	char *avatarPath = malloc(256); sprintf(avatarPath, "%splayer.json", basePath);
	FILE* file = fopen(avatarPath, "r");
	if(!file){
		printf("Failed load player file %s\n", avatarPath);
		goto avatarLoadSkip;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* content = malloc(fileSize + 1);
	int readThing = fread(content, 1, fileSize, file); (void)readThing;
	content[fileSize] = '\0';
	fclose(file);

	cJSON* json = cJSON_Parse(content);
	if(!json){
		printf("Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
		free(content);
		goto avatarLoadSkip;
	}

	cJSON* name = cJSON_GetObjectItem(json, "name");
	if(name && cJSON_IsString(name))
		newPlayer->name = strdup(name->valuestring);

	cJSON* femBody = cJSON_GetObjectItem(json, "femBody");
      if(femBody && cJSON_IsBool(femBody) && cJSON_IsTrue(femBody)){
      	DataObj* femBodyItem = newObject(&groupClass);
      	femBodyItem->name = strdup("femBody");
		parentObject(femBodyItem, newPlayer);
      }

	cJSON* colour = cJSON_GetObjectItem(json, "colour");
	if(colour && cJSON_IsArray(colour) && cJSON_GetArraySize(colour) >= 4)
		newPlayer->colour = (CharColour){
			cJSON_GetArrayItem(colour, 0)->valueint,
			cJSON_GetArrayItem(colour, 1)->valueint,
			cJSON_GetArrayItem(colour, 2)->valueint,
			cJSON_GetArrayItem(colour, 3)->valueint,
			0, COLOURMODE_RGB
		};

	cJSON* objects = cJSON_GetObjectItem(json, "objects");
	if(!objects || !cJSON_IsArray(objects)) {
		printf("No objects array found in JSON\n");
		cJSON_Delete(json);
		free(content);
		goto avatarLoadSkip;
	}
	
	printf("Found %d objects in JSON\n", cJSON_GetArraySize(objects));
	
	int objectCount = cJSON_GetArraySize(objects);
	for(int i = 0; i < objectCount; i++) {
		cJSON* obj = cJSON_GetArrayItem(objects, i);
		if(!obj)continue;
		DataObj* newObj = createObjectFromJSON(obj, newPlayer);
		if(newObj->classData->id == scriptClass.id || newObj->classData->id == playerClass.id)
		removeObject(newObj);
	}

	free(content);
avatarLoadSkip:
	free(avatarPath);
	client.gameWorld->currPlayer = newPlayer;
	return newPlayer;
}