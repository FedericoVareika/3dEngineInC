#include "obj_loading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../../utils/stb_image.h"

#include "../data_structures/array_list.h"

#define BUFFER_SIZE 1024
#define VEC_START_SIZE 0x2000
#define INDEX_START_SIZE 0x2000

#define COMMENT_CHAR '#'
#define VECTOR_CHAR 'v'
#define FACE_CHAR 'f'

static void load_v(vec3_arraylist_t *vectors, char *line) {
    char *token = strtok(line, " ");
    token = strtok(NULL, " ");
    float x = atof(token);
    token = strtok(NULL, " ");
    float y = atof(token);
    token = strtok(NULL, " ");
    float z = atof(token);

    append_vec3_al(vectors, (vec3_t){x, y, z});
    /* vectors[(*vec_counter)++] = (vec3_t){x, y, z}; */
}

static void load_vt(vec3_arraylist_t *tex_coords, char *line) {
    char *token = strtok(line, " ");
    token = strtok(NULL, " ");
    float u = atof(token);
    token = strtok(NULL, " ");
    float v = atof(token);

    append_vec3_al(tex_coords, (vec3_t){u, v, 1});
    /* tex_coords[(*vt_counter)++] = (vec3_t){u, v, 1}; */
}

static void load_vn(vec3_arraylist_t *normals, char *line) {
    char *token = strtok(line, " ");
    token = strtok(NULL, " ");
    float x = atof(token);
    token = strtok(NULL, " ");
    float y = atof(token);
    token = strtok(NULL, " ");
    float z = atof(token);

    append_vec3_al(normals, (vec3_t){x, y, z});
    /* normals[(*vn_counter)++] = (vec3_t){x, y, z}; */
}

typedef struct {
    uint_arraylist_t *v_indices;
    uint_arraylist_t *t_indices;
    uint_arraylist_t *n_indices;
} indices_t;

static int count_char(char c, char *line) {
    int i = 0;
    int count = 0;
    while (line[i] != '\0') {
        if (line[i] == c)
            count++;
        i++;
    }
    return count;
}

static void load_indices(indices_t *indices,
                         unsigned int *face_counter,
                         char *line,
                         bool *uses_t,
                         bool *uses_n) {
    int spaces = count_char(' ', line);

    char *rest = line;
    char *token = strtok_r(rest, " ", &rest);

    int *vtn_indices = malloc(sizeof(int) * spaces * 3);
    for (int i = 0; i < spaces; i++) {
        token = strtok_r(rest, " ", &rest);
        vtn_indices[i * 3 + 0] = -1;
        vtn_indices[i * 3 + 1] = -1;
        vtn_indices[i * 3 + 2] = -1;

        char *x = token;
        /* token = strpbrk(token, "/"); */
        for (int j = 0; j < 3 && token != NULL; j++) {
            token = strpbrk(token, "/");
            if (token != NULL) {
                *token = '\0';
                token++;
            }
            if (strcmp(x, ""))
                vtn_indices[i * 3 + j] = atoi(x) - 1;
            x = token;
        }
    }

    for (int i = 0; i < spaces - 2; i++) {
        // triangle i = 0, i+2, i+1
        // triangle i = 0, i+1, i+2
        int n = i + 2;
        int m = i + 1;
        cpy_uint_al(indices->v_indices,
                    (unsigned int[]){vtn_indices[0 * 3 + 0],
                                     vtn_indices[n * 3 + 0],
                                     vtn_indices[m * 3 + 0]},
                    *face_counter * 3,
                    3);
        /* memcpy(&indices->v_indices[*face_counter * 3], */
        /*        (unsigned int[]){vtn_indices[0 * 3 + 0] - 1, */
        /*                         vtn_indices[n * 3 + 0] - 1, */
        /*                         vtn_indices[m * 3 + 0] - 1}, */
        /*        sizeof(*indices->v_indices) * 3); */

        cpy_uint_al(indices->t_indices,
                    (unsigned int[]){vtn_indices[0 * 3 + 1],
                                     vtn_indices[n * 3 + 1],
                                     vtn_indices[m * 3 + 1]},
                    *face_counter * 3,
                    3);
        /* memcpy(&indices->t_indices[*face_counter * 3], */
        /*        (unsigned int[]){vtn_indices[0 * 3 + 1] - 1, */
        /*                         vtn_indices[n * 3 + 1] - 1, */
        /*                         vtn_indices[m * 3 + 1] - 1}, */
        /*        sizeof(*indices->t_indices) * 3); */
        if (vtn_indices[0 * 3 + 1] != -1 && vtn_indices[n * 3 + 1] != -1 &&
            vtn_indices[m * 3 + 1] != -1) {
            *uses_t = true;
        }

        cpy_uint_al(indices->n_indices,
                    (unsigned int[]){vtn_indices[0 * 3 + 2],
                                     vtn_indices[n * 3 + 2],
                                     vtn_indices[m * 3 + 2]},
                    *face_counter * 3,
                    3);
        /* memcpy(&indices->n_indices[*face_counter * 3], */
        /*        (unsigned int[]){vtn_indices[0 * 3 + 2] - 1, */
        /*                         vtn_indices[n * 3 + 2] - 1, */
        /*                         vtn_indices[m * 3 + 2] - 1}, */
        /*        sizeof(*indices->n_indices) * 3); */
        if (vtn_indices[0 * 3 + 2] != -1 && vtn_indices[n * 3 + 2] != -1 &&
            vtn_indices[m * 3 + 2] != -1) {
            *uses_n = true;
        }

        // v78
        printf("%d/%d/%d, %d/%d/%d, %d/%d/%d\n",
               vtn_indices[0],
               vtn_indices[1],
               vtn_indices[2],
               vtn_indices[n * 3 + 0],
               vtn_indices[n * 3 + 1],
               vtn_indices[n * 3 + 2],
               vtn_indices[m * 3 + 0],
               vtn_indices[m * 3 + 1],
               vtn_indices[m * 3 + 2]);
        (*face_counter)++;
    }

    free(vtn_indices);
}

bool load_mesh(const char *filename,
               const char *sprite_filename,
               mesh_t *mesh) {
    printf("%s\n", filename);
    fflush(stdout);
    FILE *fp;
    fp = fopen(filename, "r+");
    char buffer[BUFFER_SIZE];

    if (fp == NULL) {
        return false;
    }

    unsigned int face_counter = 0;

    vec3_arraylist_t *vectors;
    vec3_arraylist_t *tex_coords;
    vec3_arraylist_t *normals;
    vec3_arraylist_t *vec3_als = malloc(sizeof(*vectors) * 3);
    if (vec3_als == NULL) {
        printf("Error allocating vec3 arrays\n");
        return false;
    }

    vectors = &vec3_als[0];
    tex_coords = &vec3_als[1];
    normals = &vec3_als[2];

    init_vec3_al(vectors, VEC_START_SIZE);
    init_vec3_al(tex_coords, VEC_START_SIZE);
    init_vec3_al(normals, VEC_START_SIZE);

    /* vec3_t vectors[VEC_LIMIT]; */
    /* vec3_t tex_coords[VEC_LIMIT]; */
    /* vec3_t normals[VEC_LIMIT]; */

    bool uses_t = false;
    bool uses_n = false;

    uint_arraylist_t *indices_ = malloc(sizeof(*indices_) * 3);
    if (indices_ == NULL) {
        printf("Error allocating vec3 arrays\n");
        return false;
    }

    indices_t indices = {&indices_[0], &indices_[1], &indices_[2]};
    uint_arraylist_t *v_indices = &indices_[0];
    uint_arraylist_t *t_indices = &indices_[1];
    uint_arraylist_t *n_indices = &indices_[2];

    init_uint_al(v_indices, INDEX_START_SIZE);
    init_uint_al(t_indices, INDEX_START_SIZE);
    init_uint_al(n_indices, INDEX_START_SIZE);

    /* unsigned int v_indices[FACE_LIMIT * 3]; */
    /* unsigned int t_indices[FACE_LIMIT * 3]; */
    /* unsigned int n_indices[FACE_LIMIT * 3]; */

    char *line;
    while ((line = fgets(buffer, BUFFER_SIZE, fp)) != NULL) {
        switch (line[0]) {
        case '#':
        case 'g':
        case 's':
            break;
        case 'v':
            switch (line[1]) {
            case ' ':
                load_v(vectors, line);
                break;
            case 't':
                load_vt(tex_coords, line);
                break;
            case 'n':
                load_vn(normals, line);
                break;
            }
            break;
        case 'f':
            load_indices(&indices, &face_counter, line, &uses_t, &uses_n);
            break;
        case 'u':
            break;
        default:
            break;
        }
    }

    fclose(fp);

    mesh->vertices = malloc(sizeof(*mesh->vertices) * vectors->span);
    if (!mesh->vertices) {
        printf("Error allocating vertices\n");
        return false;
    }
    memcpy(
        mesh->vertices, vectors->list, sizeof(*mesh->vertices) * vectors->span);

    if (sprite_filename != NULL && tex_coords->span > 0) {
        mesh->tex_coords = malloc(sizeof(*mesh->tex_coords) * tex_coords->span);
        if (!mesh->tex_coords) {
            printf("Error allocating texture coords\n");
            return false;
        }
        memcpy(mesh->tex_coords,
               tex_coords->list,
               sizeof(*mesh->tex_coords) * tex_coords->span);
    } else
        mesh->tex_coords = NULL;

    if (normals->span > 0) {
        mesh->normals = malloc(sizeof(*mesh->normals) * normals->span);
        if (!mesh->normals) {
            printf("Error allocating vertex normals\n");
            return false;
        }
        memcpy(mesh->normals,
               normals->list,
               sizeof(*mesh->normals) * normals->span);
    } else
        mesh->normals = NULL;

    mesh->triangle_count = face_counter;

    mesh->v_indices =
        malloc(sizeof(*mesh->v_indices) * mesh->triangle_count * 3);
    if (!mesh->v_indices) {
        printf("Error allocating indices\n");
        return false;
    }
    memcpy(mesh->v_indices,
           v_indices->list,
           sizeof(*mesh->v_indices) * mesh->triangle_count * 3);

    if (sprite_filename != NULL && uses_t) {
        mesh->t_indices =
            malloc(sizeof(*mesh->t_indices) * mesh->triangle_count * 3);
        if (!mesh->t_indices) {
            printf("Error allocating indices\n");
            return false;
        }
        memcpy(mesh->t_indices,
               t_indices->list,
               sizeof(*mesh->t_indices) * mesh->triangle_count * 3);
    }

    if (uses_n) {
        mesh->n_indices =
            malloc(sizeof(*mesh->n_indices) * mesh->triangle_count * 3);
        if (!mesh->n_indices) {
            printf("Error allocating indices\n");
            return false;
        }
        memcpy(mesh->n_indices,
               n_indices->list,
               sizeof(*mesh->n_indices) * mesh->triangle_count * 3);
    }

    printf("%d vectors, %d texture coords, %d vertex normals, %d faces in %s\n",
           vectors->span,
           tex_coords->span,
           normals->span,
           face_counter,
           filename);

    free(vectors->list);
    free(tex_coords->list);
    free(normals->list);
    free(vec3_als);

    free(v_indices->list);
    free(t_indices->list);
    free(n_indices->list);
    free(indices_);

    if (sprite_filename == NULL) {
        return true;
    }

    int x, y, n, ok;
    ok = stbi_info(sprite_filename, &x, &y, &n);
    if (!ok) {
        printf("failure reason: %s\n", stbi_failure_reason());
        printf("could not get png info\n");
        return false;
    }
    stbi_set_flip_vertically_on_load(true);
    mesh->tex.data = stbi_load(sprite_filename, &x, &y, &n, 4);
    mesh->tex.n = n;
    mesh->tex.w = x;
    mesh->tex.h = y;

    return true;
}
