#include "obj_loading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define VEC_LIMIT 8192
#define FACE_LIMIT 8192

#define COMMENT_CHAR '#'
#define VECTOR_CHAR 'v'
#define FACE_CHAR 'f'

static void load_vec(vec3_t *vectors, unsigned int *vec_counter, char *line) {
    char *token = strtok(line, " ");
    token = strtok(NULL, " ");
    float x = atof(token);
    token = strtok(NULL, " ");
    float y = atof(token);
    token = strtok(NULL, " ");
    float z = atof(token);

    vectors[(*vec_counter)++] = (vec3_t){x, y, z};
}

static void
load_indices(unsigned short *indices, unsigned int *face_counter, char *line) {
    char *token = strtok(line, " ");
    token = strtok(NULL, " ");
    unsigned short x = atoi(token);
    token = strtok(NULL, " ");
    unsigned short y = atoi(token);
    token = strtok(NULL, " ");
    unsigned short z = atoi(token);

    /* printf("%i, %i, %i\n", x, y, z); */
    // TODO: change for counter-clockwise tris
    indices[*face_counter * 3 + 0] = z -1;
    indices[*face_counter * 3 + 1] = y -1;
    indices[*face_counter * 3 + 2] = x -1;
    (*face_counter)++;
}

bool load_mesh(const char *filename, mesh_t *mesh) {
    FILE *fp;
    fp = fopen(filename, "r+");
    char buffer[BUFFER_SIZE];

    if (fp == NULL) {
        return false;
    }

    unsigned int vec_counter = 0;
    unsigned int face_counter = 0;

    vec3_t vectors[VEC_LIMIT];
    unsigned short indices[FACE_LIMIT * 3];

    char *line;
    while ((line = fgets(buffer, BUFFER_SIZE, fp)) != NULL) {
        switch (line[0]) {
        case '#':
            break;
        case 'v':
            load_vec(vectors, &vec_counter, line);
            break;
        case 'f':
            load_indices(indices, &face_counter, line);
            break;
        default: 
            break;
        }
    }

    fclose(fp);

    mesh->triangle_count = face_counter;
    mesh->vertices = malloc(sizeof(vec3_t) * vec_counter);
    if (!mesh->vertices) {
        printf("Error allocating vertices\n");
        return false;
    }
    mesh->indices = malloc(sizeof(unsigned short) * mesh->triangle_count * 3);
    if (!mesh->indices) {
        printf("Error allocating indices\n");
        return false;
    }

    for (int i = 0; i < vec_counter; i++) {
        mesh->vertices[i] = vectors[i];
    }

    for (int i = 0; i < face_counter * 3; i++) {
        mesh->indices[i] = indices[i];
    }

    printf("%i vectors, %i faces in %s\n", vec_counter, face_counter, filename);

    return true;
}
