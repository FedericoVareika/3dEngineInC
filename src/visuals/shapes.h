#ifndef SHAPES_H
#define SHAPES_H

#include "../engine.h"
#include "../math/vec3.h"
#include <stdio.h>
#include <stdlib.h>

void make_unit_cube(engine_t *engine);
void make_prism(
    engine_t *engine, float x, float y, float z, float w, float h, float d);

void make_unit_cube(engine_t *engine) {
    int pos = engine->mesh_count;
    engine->meshes[pos].vertices = malloc(sizeof(vec3_t) * 36);
    engine->meshes[pos].vertex_count = 36; // find a way to make it 8
    engine->meshes[pos].triangle_count = 12;
    if (!engine->meshes[pos].vertices) {
        fprintf(stderr, "Vertices malloc failed");
        return;
    }

    vec3_t *vertices = engine->meshes[pos].vertices;

    // South wall
    vertices[0] = (vec3_t){-0.5, -0.5, -4.5}; // left bottom near
    vertices[1] = (vec3_t){-0.5, 0.5, -4.5};  // left top near
    vertices[2] = (vec3_t){0.5, 0.5, -4.5};   // right top near

    vertices[3] = (vec3_t){-0.5, -0.5, -4.5}; // left bottom near
    vertices[4] = (vec3_t){0.5, 0.5, -4.5};   // right top near
    vertices[5] = (vec3_t){0.5, -0.5, -4.5};  // right bottom near

    // North wall
    vertices[6] = (vec3_t){-0.5, -0.5, -5.5}; // left bottom far
    vertices[7] = (vec3_t){0.5, 0.5, -5.5};   // right top far
    vertices[8] = (vec3_t){-0.5, 0.5, -5.5};  // left top far

    vertices[9] = (vec3_t){-0.5, -0.5, -5.5}; // left bottom far
    vertices[10] = (vec3_t){0.5, -0.5, -5.5}; // right bottom far
    vertices[11] = (vec3_t){0.5, 0.5, -5.5};  // right top far

    // Down wall
    vertices[12] = (vec3_t){-0.5, -0.5, -4.5}; // left bottom near
    vertices[13] = (vec3_t){0.5, -0.5, -4.5};  // right bottom near
    vertices[14] = (vec3_t){0.5, -0.5, -5.5};  // right bottom far

    vertices[15] = (vec3_t){-0.5, -0.5, -4.5}; // left bottom near
    vertices[16] = (vec3_t){0.5, -0.5, -5.5};  // right bottom far
    vertices[17] = (vec3_t){-0.5, -0.5, -5.5}; // left bottom far

    // Top wall
    vertices[18] = (vec3_t){-0.5, 0.5, -4.5}; // left top near
    vertices[19] = (vec3_t){0.5, 0.5, -5.5};  // right top far
    vertices[20] = (vec3_t){0.5, 0.5, -4.5};  // right top near

    vertices[21] = (vec3_t){-0.5, 0.5, -4.5}; // left top near
    vertices[22] = (vec3_t){-0.5, 0.5, -5.5}; // left top far
    vertices[23] = (vec3_t){0.5, 0.5, -5.5};  // right top far

    // Left wall
    vertices[24] = (vec3_t){-0.5, -0.5, -5.5}; // left bottom far
    vertices[25] = (vec3_t){-0.5, 0.5, -4.5};  // left top near
    vertices[26] = (vec3_t){-0.5, -0.5, -4.5}; // left bottom near

    vertices[27] = (vec3_t){-0.5, -0.5, -5.5}; // left bottom far
    vertices[28] = (vec3_t){-0.5, 0.5, -5.5};  // left top far
    vertices[29] = (vec3_t){-0.5, 0.5, -4.5};  // left top near

    // Right wall
    vertices[30] = (vec3_t){0.5, -0.5, -4.5}; // right bottom near
    vertices[31] = (vec3_t){0.5, 0.5, -4.5};  // right top near
    vertices[32] = (vec3_t){0.5, 0.5, -5.5};  // right top far

    vertices[33] = (vec3_t){0.5, -0.5, -4.5}; // right bottom near
    vertices[34] = (vec3_t){0.5, 0.5, -5.5};  // right top far
    vertices[35] = (vec3_t){0.5, -0.5, -5.5}; // right bottom far

    engine->mesh_count++;
}

void make_prism(
    engine_t *engine, float x, float y, float z, float w, float h, float d) {
    int pos = engine->mesh_count;
    engine->meshes[pos].vertices = malloc(sizeof(vec3_t) * 36);
    engine->meshes[pos].vertex_count = 36; // find a way to make it 8
    engine->meshes[pos].triangle_count = 12;
    if (!engine->meshes[pos].vertices) {
        fprintf(stderr, "Vertices malloc failed");
        return;
    }

    vec3_t *vertices = engine->meshes[pos].vertices;

    // near
    vec3_t v0 = {x, y, z};         // left bottom near
    vec3_t v1 = {x, y + h, z};     // left top near
    vec3_t v2 = {x + w, y + h, z}; // right top near
    vec3_t v3 = {x + w, y, z};     // right bottom near

    // far
    vec3_t v4 = {x, y, z - d};         // left bottom far
    vec3_t v5 = {x, y + h, z - d};     // left top far
    vec3_t v6 = {x + w, y + h, z - d}; // right top far
    vec3_t v7 = {x + w, y, z - d};     // right bottom far

    // South wall
    vertices[0] = v0; // left bottom near
    vertices[1] = v1; // left top near
    vertices[2] = v2; // right top near

    vertices[3] = v0; // left bottom near
    vertices[4] = v2; // right top near
    vertices[5] = v3; // right bottom near

    // North wall
    vertices[6] = v4; // left bottom far
    vertices[7] = v6; // right top far
    vertices[8] = v5; // left top far

    vertices[9] = v4;  // left bottom far
    vertices[10] = v7; // right bottom far
    vertices[11] = v6; // right top far

    // Down wall
    vertices[12] = v0; // left bottom near
    vertices[13] = v3; // right bottom near
    vertices[14] = v7; // right bottom far

    vertices[15] = v0; // left bottom near
    vertices[16] = v7; // right bottom far
    vertices[17] = v4; // left bottom far

    // Top wall
    vertices[18] = v1; // left top near
    vertices[19] = v6; // right top far
    vertices[20] = v2; // right top near

    vertices[21] = v1; // left top near
    vertices[22] = v5; // left top far
    vertices[23] = v6; // right top far

    // Left wall
    vertices[24] = v4; // left bottom far
    vertices[25] = v1; // left top near
    vertices[26] = v0; // left bottom near

    vertices[27] = v4; // left bottom far
    vertices[28] = v5; // left top far
    vertices[29] = v1; // left top near

    // Right wall
    vertices[30] = v3; // right bottom near
    vertices[31] = v2; // right top near
    vertices[32] = v6; // right top far

    vertices[33] = v3; // right bottom near
    vertices[34] = v6; // right top far
    vertices[35] = v7; // right bottom far

    engine->mesh_count++;
}

#endif // !SHAPES_H
