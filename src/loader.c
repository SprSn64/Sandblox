//load txt and obj/custom binary mesh files functions

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "loader.h"

typedef struct {
    int v;
    int vt;
    int vn;
} FaceIndex;


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

Mesh *loadMeshFromObj(const char *path) {
    Uint32 vcount = 0, vtcount = 0, vncount = 0, tricount = 0;
    objCount(path, &vcount, &vtcount, &vncount, &tricount);

    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    Mesh *mesh = calloc(1, sizeof(Mesh));
    mesh->vertCount = vcount;
    mesh->faceCount = tricount;
    mesh->verts = calloc(vcount, sizeof(MeshVert));
    mesh->faces = calloc(tricount, sizeof(MeshFace));

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
                face->vertA = a;
                face->vertB = b;
                face->vertC = c;
            }
        }
    }

    fclose(f);
    free(uvs);
    free(normals);
    return mesh;
}
