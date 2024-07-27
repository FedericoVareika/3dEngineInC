#include "array_list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// --------------------------uint arraylist-----------------------------------
// ---------------------------------------------------------------------------

void init_uint_al(uint_arraylist_t *al, unsigned int start_len) {
    assert(al != NULL);
    assert(start_len > 0);
    al->list = malloc(sizeof(*al->list) * start_len);
    al->span = 0;
    al->length = start_len;
}

void resize_uint_al(uint_arraylist_t *al) {
    assert(al != NULL);
    al->list = realloc(al->list, sizeof(*al->list) * al->length * 2);
    assert(al != NULL);
    al->length = al->length * 2;
}

void modify_uint_al(uint_arraylist_t *al, unsigned int val, unsigned int at) {
    assert(al != NULL);
    while (at >= al->length) {
        resize_uint_al(al);
    }
    al->list[at] = val;
    al->span = al->span < at ? at : al->span;
}

void cpy_uint_al(uint_arraylist_t *al,
                 unsigned int *src,
                 unsigned int at,
                 unsigned int len) {
    assert(al != NULL);
    while (at + len > al->length) {
        resize_uint_al(al);
    }
    memcpy(&al->list[at], src, sizeof(*al->list) * len);
    al->span = al->span < at + len - 1 ? at + len - 1 : al->span;
}

void append_uint_al(uint_arraylist_t *al, unsigned int val) {
    assert(al != NULL);
    if (al->length == al->span) {
        resize_uint_al(al);
    }
    al->list[al->span] = val;
    al->span++;
}

void destroy_uint_al(uint_arraylist_t *al) {
    assert(al != NULL);
    free(al->list);
    free(al);
}

// ---------------------------------------------------------------------------
// --------------------------vec3 arraylist-----------------------------------
// ---------------------------------------------------------------------------

void init_vec3_al(vec3_arraylist_t *al, unsigned int start_len) {
    assert(al != NULL);
    assert(start_len > 0);
    al->list = malloc(sizeof(*al->list) * start_len);
    al->span = 0;
    al->length = start_len;
}

void resize_vec3_al(vec3_arraylist_t *al) {
    assert(al != NULL);
    al->list = realloc(al->list, sizeof(*al->list) * al->length * 2);
    assert(al != NULL);
    al->length = al->length * 2;
}

void modify_vec3_al(vec3_arraylist_t *al, vec3_t val, unsigned int at) {
    assert(al != NULL);
    while (at >= al->length) {
        resize_vec3_al(al);
    }
    al->list[at] = val;
    al->span = al->span < at ? at : al->span;
}

void cpy_vec3_al(vec3_arraylist_t *al,
                 vec3_t *src,
                 unsigned int at,
                 unsigned int len) {
    assert(al != NULL);
    while (at + len > al->length) {
        resize_vec3_al(al);
    }
    memcpy(&al->list[at], src, sizeof(*al->list) * len);
    al->span = al->span < at + len - 1 ? at + len - 1 : al->span;
}

void append_vec3_al(vec3_arraylist_t *al, vec3_t val) {
    assert(al != NULL);
    if (al->length == al->span) {
        resize_vec3_al(al);
    }
    al->list[al->span] = val;
    al->span++;
}

void destroy_vec3_al(vec3_arraylist_t *al) {
    assert(al != NULL);
    free(al->list);
    free(al);
}
