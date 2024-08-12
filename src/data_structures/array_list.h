#pragma once

typedef struct {
    unsigned int *list;
    unsigned int span;
    unsigned int length;
} uint_arraylist_t;

void init_uint_al(uint_arraylist_t *al, unsigned int start_len);
void modify_uint_al(uint_arraylist_t *al, unsigned int val, unsigned int at);
void cpy_uint_al(uint_arraylist_t *al,
                 unsigned int *src,
                 unsigned int at,
                 unsigned int len);
void append_uint_al(uint_arraylist_t *al, unsigned int val);
void destroy_uint_al(uint_arraylist_t *al);

#include "../math/vec3.h"

typedef struct {
    vec3_t *list;
    unsigned int span;
    unsigned int length;
} vec3_arraylist_t;

void init_vec3_al(vec3_arraylist_t *al, unsigned int start_len);
void modify_vec3_al(vec3_arraylist_t *al, vec3_t val, unsigned int at);
void append_vec3_al(vec3_arraylist_t *al, vec3_t val);
void destroy_vec3_al(vec3_arraylist_t *al);

#include "../engine.h"

typedef struct {
    mtl_t *list;
    unsigned int span;
    unsigned int length;
} mtl_arraylist_t;

void init_mtl_al(mtl_arraylist_t *al, unsigned int start_len);
void modify_mtl_al(mtl_arraylist_t *al, mtl_t val, unsigned int at);
void append_mtl_al(mtl_arraylist_t *al, mtl_t *val);
void destroy_mtl_al(mtl_arraylist_t *al);

typedef struct {
    tex_t *list;
    unsigned int span;
    unsigned int length;
} tex_arraylist_t;

void init_tex_al(tex_arraylist_t *al, unsigned int start_len);
void modify_tex_al(tex_arraylist_t *al, tex_t val, unsigned int at);
void append_tex_al(tex_arraylist_t *al, tex_t *val);
void destroy_tex_al(tex_arraylist_t *al);

typedef struct {
    mesh_t *list;
    unsigned int span;
    unsigned int length;
} mesh_arraylist_t;

void init_mesh_al(mesh_arraylist_t *al, unsigned int start_len);
void modify_mesh_al(mesh_arraylist_t *al, mesh_t val, unsigned int at);
void append_mesh_al(mesh_arraylist_t *al, mesh_t *val);
void destroy_mesh_al(mesh_arraylist_t *al);
