#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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