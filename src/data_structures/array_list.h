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
