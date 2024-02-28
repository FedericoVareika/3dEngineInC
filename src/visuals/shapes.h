#ifndef SHAPES_H
#define SHAPES_H

#include "../engine.h"
#include "../math/vec3.h"
#include <stdio.h>
#include <stdlib.h>

void make_unit_cube(engine_t *engine);
void make_prism(
    engine_t *engine, float x, float y, float z, float w, float h, float d);
void make_triangle(engine_t *engine);

void make_unit_cube(engine_t *engine) {
    int vertex_count = 8;
    int triangle_count = 12;
    int index_count = triangle_count * 3;

    int pos = engine->mesh_count;
    engine->meshes[pos]->vertices = malloc(sizeof(vec3_t) * vertex_count);
    if (!engine->meshes[pos]->vertices) {
        fprintf(stderr, "Vertices malloc failed");
        return;
    }

    engine->meshes[pos]->indices = malloc(sizeof(unsigned short) * index_count);
    if (!engine->meshes[pos]->indices) {
        fprintf(stderr, "Indices malloc failed");
        return;
    }

    engine->meshes[pos]->vertex_count = vertex_count;
    engine->meshes[pos]->triangle_count = triangle_count;

    vec3_t *vertices = engine->meshes[pos]->vertices;

    // near
    vertices[0] = (vec3_t){0, 0, 0}; // left bottom near
    vertices[1] = (vec3_t){0, 1, 0}; // left top near
    vertices[2] = (vec3_t){1, 1, 0}; // right top near
    vertices[3] = (vec3_t){1, 0, 0}; // right bottom near

    // far
    vertices[4] = (vec3_t){0, 0, -1}; // left bottom far
    vertices[5] = (vec3_t){0, 1, -1}; // left top far
    vertices[6] = (vec3_t){1, 1, -1}; // right top far
    vertices[7] = (vec3_t){1, 0, -1}; // right bottom far

    unsigned short *indices = (unsigned short[36]){
        0, 1, 2, 0, 2, 3, // south wall
        7, 6, 5, 4, 7, 5, // north wall
        4, 0, 3, 4, 3, 7, // down wall
        1, 5, 6, 1, 6, 2, // top wall
        4, 5, 1, 4, 1, 0, // left wall
        3, 2, 6, 3, 6, 7, // right wall
    };

    for (int i = 0; i < index_count; i++) {
        engine->meshes[pos]->indices[i] = indices[i];
    }
    engine->mesh_count++;
}

void make_prism(
    engine_t *engine, float x, float y, float z, float w, float h, float d) {
    int vertex_count = 8;
    int triangle_count = 12;
    int index_count = triangle_count * 3;

    int pos = engine->mesh_count;
    engine->meshes[pos]->vertices = malloc(sizeof(vec3_t) * vertex_count); // HERE
    if (!engine->meshes[pos]->vertices) {
        fprintf(stderr, "Vertices malloc failed");
        return;
    }

    engine->meshes[pos]->indices = malloc(sizeof(unsigned short) * index_count);
    if (!engine->meshes[pos]->indices) {
        fprintf(stderr, "Indices malloc failed");
        return;
    }

    engine->meshes[pos]->vertex_count = vertex_count;
    engine->meshes[pos]->triangle_count = triangle_count;

    vec3_t *vertices = engine->meshes[pos]->vertices;

    // near
    vertices[0] = (vec3_t){x, y, z};         // left bottom near
    vertices[1] = (vec3_t){x, y + h, z};     // left top near
    vertices[2] = (vec3_t){x + w, y + h, z}; // right top near
    vertices[3] = (vec3_t){x + w, y, z};     // right bottom near

    // far
    vertices[4] = (vec3_t){x, y, z - d};         // left bottom far
    vertices[5] = (vec3_t){x, y + h, z - d};     // left top far
    vertices[6] = (vec3_t){x + w, y + h, z - d}; // right top far
    vertices[7] = (vec3_t){x + w, y, z - d};     // right bottom far

    // unsigned short *indices = engine->meshes[pos].indices;

    unsigned short *indices = (unsigned short[36]){
        0, 1, 2, 0, 2, 3, // south wall
        7, 6, 5, 4, 7, 5, // north wall
        4, 0, 3, 4, 3, 7, // down wall
        1, 5, 6, 1, 6, 2, // top wall
        4, 5, 1, 4, 1, 0, // left wall
        3, 2, 6, 3, 6, 7, // right wall
    };

    for (int i = 0; i < index_count; i++) {
        engine->meshes[pos]->indices[i] = indices[i];
    }

    engine->mesh_count++;
}

void make_triangle(engine_t *engine) {
    int vertex_count = 3;
    int triangle_count = 1;

    int index_count = triangle_count * 3;

    int pos = engine->mesh_count;
    engine->meshes[pos]->vertices = malloc(sizeof(vec3_t) * vertex_count);
    if (!engine->meshes[pos]->vertices) {
        fprintf(stderr, "Vertices malloc failed");
        return;
    }

    engine->meshes[pos]->indices = malloc(sizeof(unsigned short) * index_count);
    if (!engine->meshes[pos]->indices) {
        fprintf(stderr, "Indices malloc failed");
        return;
    }

    engine->meshes[pos]->vertex_count = vertex_count;
    engine->meshes[pos]->triangle_count = triangle_count;

    vec3_t *vertices = engine->meshes[pos]->vertices;

    vertices[0] = (vec3_t){0, 0, 0}; // left bottom near
    vertices[1] = (vec3_t){0, 1, 0}; // left top near
    vertices[2] = (vec3_t){1, 1, 0}; // right top near

    engine->meshes[pos]->indices = (unsigned short[3]){0, 1, 2};
    engine->mesh_count++;
}

#endif // !SHAPES_H
