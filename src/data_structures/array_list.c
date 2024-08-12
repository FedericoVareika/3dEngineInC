#include "array_list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define init_al(al, start_len)                                                 \
    ({                                                                         \
        assert(al != NULL);                                                    \
        assert(start_len > 0);                                                 \
        al->list = malloc(sizeof(*al->list) * start_len);                      \
        al->span = 0;                                                          \
        al->length = start_len;                                                \
    })

#define resize_al(al)                                                          \
    ({                                                                         \
        assert(al != NULL);                                                    \
        al->list = realloc(al->list, sizeof(*al->list) * al->length * 2);      \
        assert(al != NULL);                                                    \
        al->length = al->length * 2;                                           \
    })

#define modify_al(al, val, at)                                                 \
    ({                                                                         \
        assert(al != NULL);                                                    \
        while (at >= al->length) resize_al(al);                                \
        al->list[at] = val;                                                    \
        al->span = al->span < at ? at : al->span;                              \
    })

#define cpy_al(al, src, at, len)                                               \
    ({                                                                         \
        assert(al != NULL);                                                    \
        while (at + len > al->length) resize_al(al);                           \
        memcpy(&al->list[at], src, sizeof(*al->list) * len);                   \
        al->span = al->span < at + len ? at + len : al->span;                  \
    })

#define append_al(al, val)                                                     \
    ({                                                                         \
        assert(al != NULL);                                                    \
        if (al->length == al->span) resize_al(al);                             \
        al->list[al->span] = val;                                              \
        al->span++;                                                            \
    })

#define destroy_al(al)                                                         \
    ({                                                                         \
        assert(al != NULL);                                                    \
        free(al->list);                                                        \
        free(al);                                                              \
    })

// ---------------------------------------------------------------------------
// --------------------------uint arraylist-----------------------------------
// ---------------------------------------------------------------------------

void init_uint_al(uint_arraylist_t *al, unsigned int start_len) {
    init_al(al, start_len);
}

void resize_uint_al(uint_arraylist_t *al) {
    resize_al(al);
}

void modify_uint_al(uint_arraylist_t *al, unsigned int val, unsigned int at) {
    modify_al(al, val, at);
}

void cpy_uint_al(uint_arraylist_t *al, unsigned int *src, unsigned int at,
                 unsigned int len) {
    cpy_al(al, src, at, len);
}

void append_uint_al(uint_arraylist_t *al, unsigned int val) {
    append_al(al, val);
}

void destroy_uint_al(uint_arraylist_t *al) {
    destroy_al(al);
}

// ---------------------------------------------------------------------------
// --------------------------vec3 arraylist-----------------------------------
// ---------------------------------------------------------------------------

void init_vec3_al(vec3_arraylist_t *al, unsigned int start_len) {
    init_al(al, start_len);
}

void resize_vec3_al(vec3_arraylist_t *al) {
    resize_al(al);
}

void modify_vec3_al(vec3_arraylist_t *al, vec3_t val, unsigned int at) {
    modify_al(al, val, at);
}

void cpy_vec3_al(vec3_arraylist_t *al, vec3_t *src, unsigned int at,
                 unsigned int len) {
    cpy_al(al, src, at, len);
}

void append_vec3_al(vec3_arraylist_t *al, vec3_t val) {
    append_al(al, val);
}

void destroy_vec3_al(vec3_arraylist_t *al) {
    destroy_al(al);
}

// ---------------------------------------------------------------------------
// --------------------------mtl arraylist------------------------------------
// ---------------------------------------------------------------------------

void init_mtl_al(mtl_arraylist_t *al, unsigned int start_len) {
    init_al(al, start_len);
}

void resize_mtl_al(mtl_arraylist_t *al) {
    resize_al(al);
}

void modify_mtl_al(mtl_arraylist_t *al, mtl_t val, unsigned int at) {
    modify_al(al, val, at);
}

void cpy_mtl_al(mtl_arraylist_t *al, mtl_t *src, unsigned int at,
                unsigned int len) {
    cpy_al(al, src, at, len);
}

void append_mtl_al(mtl_arraylist_t *al, mtl_t *val) {
    append_al(al, *val);
}

void destroy_mtl_al(mtl_arraylist_t *al) {
    destroy_al(al);
}

// ---------------------------------------------------------------------------
// --------------------------tex arraylist------------------------------------
// ---------------------------------------------------------------------------

void init_tex_al(tex_arraylist_t *al, unsigned int start_len) {
    init_al(al, start_len);
}

void modify_tex_al(tex_arraylist_t *al, tex_t val, unsigned int at) {
    modify_al(al, val, at);
}

void append_tex_al(tex_arraylist_t *al, tex_t *val) {
    append_al(al, *val);
}

void destroy_tex_al(tex_arraylist_t *al) {
    destroy_al(al);
}

// ---------------------------------------------------------------------------
// --------------------------mesh arraylist-----------------------------------
// ---------------------------------------------------------------------------

void init_mesh_al(mesh_arraylist_t *al, unsigned int start_len) {
    init_al(al, start_len);
}

void modify_mesh_al(mesh_arraylist_t *al, mesh_t val, unsigned int at) {
    modify_al(al, val, at);
}

void append_mesh_al(mesh_arraylist_t *al, mesh_t *val) {
    append_al(al, *val);
}

void destroy_mesh_al(mesh_arraylist_t *al) {
    destroy_al(al);
}
