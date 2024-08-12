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
#define MESH_START_SIZE 0x5
#define MTL_START_SIZE 0x5
#define TEX_START_SIZE 0x5

typedef enum {
    // general purpose codes
    NULL_CODE,
    EMPTY,
    COMMENT,
    // .obj file codes
    MTL_LIB,
    USE_MTL,
    OBJECT,
    GROUP,
    VERTEX,
    VERTEX_NORMAL,
    VERTEX_TEX,
    FACE,
    // .mtl file codes
    NEW_MTL,
    KA,
    KD,
    KS,
    NS,
    D,
    NI,
    ILLUM,
    MAP_KA,
    MAP_KD,
    MAP_KS,
} LINE_CODE;

char *get_filepath(const char *path, const char *filename) {
    int len_path = strlen(path);
    int len_filename = strlen(filename);

    char *filepath = malloc(sizeof(*filepath) * (len_path + len_filename + 1));

    int j = 0;
    int i = 0;
    while (path[j] != '\0') {
        if (path[j] != '\n' && path[j] != '\r') { filepath[i++] = path[j]; }
        j++;
    }
    j = 0;
    while (filename[j] != '\0') {
        if (filename[j] != '\n' && filename[j] != '\r') {
            filepath[i++] = filename[j];
        }
        j++;
    }
    filepath[i] = '\0';

    return filepath;
}

void parse_single_float(float *float_out, char *line_in) {
    assert(line_in);
    char *token = strtok(line_in, " ");
    *float_out = atof(token);
}

void parse_triple_float(float triple_out[3], char *line_in) {
    assert(line_in);
    char *token = strtok(line_in, " ");
    triple_out[0] = atof(token);
    token = strtok(NULL, " ");
    triple_out[1] = atof(token);
    token = strtok(NULL, " ");
    triple_out[2] = atof(token);
}

void load_tex(unsigned int *idx_out, tex_arraylist_t *txts_out, char *line_in,
              const char *path) {
    assert(line_in);
    assert(txts_out);
    while (line_in[0] && (line_in[0] == ' ' || line_in[0] == '\t')) line_in++;
    int len = 0;
    char *end_name_ptr = line_in;
    while (end_name_ptr[0] && end_name_ptr[0] != ' ' &&
           end_name_ptr[0] != '\t' && end_name_ptr[0] != '\n' &&
           end_name_ptr[0] != '\r') {
        len++;
        end_name_ptr++;
    }
    *end_name_ptr = '\0';

    *idx_out = 0;
    char *tex_filename = line_in;
    tex_t *al_ptr = txts_out->list;
    while (*idx_out < txts_out->span && al_ptr) {
        if (strncmp(tex_filename, al_ptr->name, len) == 0) return;
        (*idx_out)++;
        al_ptr++;
    }

    char *tex_filepath = get_filepath(path, tex_filename);
    tex_filename = strdup(tex_filename);
    int x, y, n, ok;
    ok = stbi_info(tex_filepath, &x, &y, &n);
    if (!ok) {
        printf("failure reason: %s\n", stbi_failure_reason());
        printf("could not get texture info: %.*s\n", len, tex_filepath);
        free(tex_filepath);
        return;
    }

    stbi_set_flip_vertically_on_load(true);
    tex_t *new_tex = malloc(sizeof(*new_tex));
    if (!new_tex) printf("Error allocating texture");
    new_tex->name = tex_filename;
    new_tex->data = stbi_load(tex_filepath, &x, &y, &n, 4);
    new_tex->n = n;
    new_tex->w = x;
    new_tex->h = y;

    free(tex_filepath);

    append_tex_al(txts_out, new_tex);
    free(new_tex);
}

LINE_CODE get_line_code(char *line) {
    assert(line);
    while (line[0] != '\0' && (line[0] == '\n' || line[0] == '\r' ||
                               line[0] == '\t' || line[0] == ' '))
        line++;

    if (line[0] == '\0') return EMPTY;
    if (line[0] == '#') return COMMENT;

    // .obj file line codes
    if (!strncmp(line, "v ", 2)) return VERTEX;
    if (!strncmp(line, "vn ", 3)) return VERTEX_NORMAL;
    if (!strncmp(line, "vt ", 3)) return VERTEX_TEX;

    if (!strncmp(line, "o ", 2)) return OBJECT;
    if (!strncmp(line, "g ", 2)) return GROUP;

    if (!strncmp(line, "f ", 2)) return FACE;

    if (!strncmp(line, "mtllib ", 7)) return MTL_LIB;
    if (!strncmp(line, "usemtl ", 7)) return USE_MTL;

    // .mtl file line codes
    if (!strncmp(line, "newmtl ", 7)) return NEW_MTL;

    if (!strncmp(line, "Ka ", 3)) return KA;
    if (!strncmp(line, "Kd ", 3)) return KD;
    if (!strncmp(line, "Ks ", 3)) return KS;

    if (!strncmp(line, "Ns ", 3)) return NS;
    if (!strncmp(line, "d ", 2)) return D;
    if (!strncmp(line, "Ni ", 3)) return NI;
    if (!strncmp(line, "illum ", 6)) return ILLUM;

    if (!strncmp(line, "map_Ka ", 7)) return MAP_KA;
    if (!strncmp(line, "map_Kd ", 7)) return MAP_KD;
    if (!strncmp(line, "map_Ks ", 7)) return MAP_KS;

    return NULL_CODE;
}

int load_mtls(mtl_arraylist_t *mtls_out, tex_arraylist_t *texs_out,
              char *mtllib_in, const char *path) {
    assert(mtllib_in);

    FILE *mtlfp;
    char *mtllib_path = get_filepath(path, mtllib_in);
    mtlfp = fopen(mtllib_path, "r+");
    if (!mtlfp) {
        printf("Error opening file: %s\n", mtllib_path);
        free(mtllib_path);
        return 0;
    }
    free(mtllib_path);

    mtl_t *current_mtl = NULL;

    char line[BUFFER_SIZE];
    while (fgets(line, BUFFER_SIZE, mtlfp) != NULL) {
        LINE_CODE code = get_line_code(line);
        switch (code) {
        case NULL_CODE:
        case EMPTY:
        case COMMENT:
            break;

        case NEW_MTL:
            if (current_mtl != NULL)
                append_mtl_al(mtls_out, current_mtl);
            else
                current_mtl = malloc(sizeof(*current_mtl));
            char *name = &line[7];
            int i = 0;
            while (name[i] != '\0' && name[i] != '\n' && name[i] != '\r') i++;
            name[i] = '\0';
            current_mtl->name = strdup(name);
            current_mtl->ambient_tex_idx = -1;
            current_mtl->diffuse_tex_idx = -1;
            current_mtl->specular_tex_idx = -1;
            break;

        case KA:
            parse_triple_float(current_mtl->ambient, &line[3]);
            break;
        case KD:
            parse_triple_float(current_mtl->diffuse, &line[3]);
            break;
        case KS:
            parse_triple_float(current_mtl->specular, &line[3]);
            break;

        case NS:
            parse_single_float(&current_mtl->specular_exp, &line[3]);
            break;
        case D:
            parse_single_float(&current_mtl->dissolved, &line[2]);
            break;
        case NI:
            parse_single_float(&current_mtl->optical_density, &line[3]);
            break;
        case ILLUM:
            parse_single_float(&current_mtl->illum, &line[6]);
            break;

        case MAP_KA: {
            unsigned int idx;
            load_tex(&idx, texs_out, &line[7], path);
            current_mtl->ambient_tex_idx = (unsigned int)idx;
        } break;
        case MAP_KD: {
            unsigned int idx;
            load_tex(&idx, texs_out, &line[7], path);
            current_mtl->diffuse_tex_idx = (unsigned int)idx;
        } break;
        case MAP_KS: {
            unsigned int idx;
            load_tex(&idx, texs_out, &line[7], path);
            current_mtl->specular_tex_idx = (unsigned int)idx;
        } break;
        default:
            break;
        }
    }

    if (current_mtl != NULL) {
        append_mtl_al(mtls_out, current_mtl);
        free(current_mtl);
    }

    fclose(mtlfp);

    return 1;
}

void set_mtl(mesh_t *mesh_out, mtl_t *mtls_in, int mtls_count, char *line_in) {
    assert(mesh_out);
    assert(mtls_in);
    assert(line_in);

    char *mtl_name = line_in;
    int len = 0;
    while (mtl_name[len] && mtl_name[len] != '\n' && mtl_name[len] != '\r')
        len++;
    mtl_name[len] = '\0';

    int i;
    for (i = 0; i < mtls_count; i++) {
        if (strcmp(mtls_in[i].name, mtl_name) == 0) break;
    }
    mtls_in = i == mtls_count ? NULL : &mtls_in[i];

    if (mtls_in)
        mesh_out->mtl = mtls_in;
    else
        printf("Could not find mtl with name: %s", line_in);
}

void load_v(vec3_arraylist_t *vectors_out, char *line_in) {
    assert(vectors_out);
    assert(line_in);
    vec3_t vec = {0, 0, 1};
    char *token = strtok(line_in, " ");
    if (!token) goto end_load_v;
    vec.x = atof(token);
    token = strtok(NULL, " ");
    if (!token) goto end_load_v;
    vec.y = atof(token);
    token = strtok(NULL, " ");
    if (!token) goto end_load_v;
    vec.z = atof(token);
end_load_v:
    append_vec3_al(vectors_out, vec);
}

typedef struct {
    uint_arraylist_t *v_indices;
    uint_arraylist_t *t_indices;
    uint_arraylist_t *n_indices;
} indices_t;

int atoi_n(char *str, unsigned int n) {
    assert(str);
    if (n == 0) return -1;

    int i = 0;
    int res = 0;
    while (i < n) {
        if (!str[i] || str[i] < '0' || str[i] > '9') return -1;
        res *= 10;
        res += str[i] - '0';
        i++;
    }
    return res;
}

void set_mesh_indices(mesh_t *mesh_out, indices_t *indices_in) {
    unsigned int *v_indices =
        malloc(sizeof(*v_indices) * indices_in->v_indices->span);
    unsigned int *n_indices =
        malloc(sizeof(*n_indices) * indices_in->n_indices->span);
    unsigned int *t_indices =
        malloc(sizeof(*t_indices) * indices_in->t_indices->span);

    memcpy(v_indices, indices_in->v_indices->list,
           sizeof(*indices_in->v_indices->list) * indices_in->v_indices->span);
    memcpy(n_indices, indices_in->n_indices->list,
           sizeof(*indices_in->n_indices->list) * indices_in->n_indices->span);
    memcpy(t_indices, indices_in->t_indices->list,
           sizeof(*indices_in->t_indices->list) * indices_in->t_indices->span);

    mesh_out->v_indices = v_indices;
    mesh_out->n_indices = n_indices;
    mesh_out->t_indices = t_indices;
}

void reset_indices(indices_t *indices_out) {
    free(indices_out->v_indices->list);
    free(indices_out->n_indices->list);
    free(indices_out->t_indices->list);
    init_uint_al(indices_out->v_indices, INDEX_START_SIZE);
    init_uint_al(indices_out->n_indices, INDEX_START_SIZE);
    init_uint_al(indices_out->t_indices, INDEX_START_SIZE);
}

void load_index(unsigned int *v_out, unsigned int *n_out, unsigned int *t_out,
                char **index_in) {
    assert(v_out);
    assert(n_out);
    assert(t_out);
    assert(index_in);

    while (*index_in && (*index_in[0] == ' ' || *index_in[0] == '\t'))
        (*index_in)++;

    *v_out = -1;
    *n_out = -1;
    *t_out = -1;

    int len = 0;
    while ((*index_in)[len] != '\0' && (*index_in)[len] != '/' &&
           (*index_in)[len] != ' ' && (*index_in)[len] != '\n' &&
           (*index_in)[len] != '\r')
        len++;
    *v_out = atoi_n(*index_in, len) - 1;

    char index_char = (*index_in)[len];
    *index_in = (*index_in)[len] ? &(*index_in)[len + 1] : &(*index_in)[len];
    if (index_char != '/') return;

    len = 0;
    while ((*index_in)[len] != '\0' && (*index_in)[len] != '/' &&
           (*index_in)[len] != ' ' && (*index_in)[len] != '\n' &&
           (*index_in)[len] != '\r')
        len++;
    *t_out = atoi_n(*index_in, len) - 1;

    index_char = (*index_in)[len];
    *index_in = (*index_in)[len] ? &(*index_in)[len + 1] : &(*index_in)[len];
    if (index_char != '/') return;

    len = 0;
    while ((*index_in)[len] != '\0' && (*index_in)[len] != '/' &&
           (*index_in)[len] != ' ' && (*index_in)[len] != '\n' &&
           (*index_in)[len] != '\r')
        len++;
    *n_out = atoi_n(*index_in, len) - 1;

    *index_in = &(*index_in)[len];
}

void load_face(indices_t *indices_out, unsigned int *face_counter_out,
               char *line_in) {
    assert(indices_out);
    assert(face_counter_out);
    assert(line_in);

    uint_arraylist_t *vectors = malloc(sizeof(*vectors));
    assert(vectors);
    init_uint_al(vectors, 10);
    uint_arraylist_t *normals = malloc(sizeof(*normals));
    assert(normals);
    init_uint_al(normals, 10);
    uint_arraylist_t *tex_coords = malloc(sizeof(*tex_coords));
    assert(tex_coords);
    init_uint_al(tex_coords, 10);

    unsigned int v;
    unsigned int n;
    unsigned int t;

    while (line_in[0] != '\0' && line_in[0] != '\n' && line_in[0] != '\r') {
        load_index(&v, &n, &t, &line_in);
        append_uint_al(vectors, v);
        append_uint_al(normals, n);
        append_uint_al(tex_coords, t);
    }

    for (int i = 0; i < vectors->span - 2; i++) {
        bool clockwise = false;
        int n = i + 2;
        int m = i + 1;
        if (clockwise) {
            n = i + 1;
            m = i + 2;
        }

        cpy_uint_al(indices_out->v_indices,
                    (unsigned int[3]){vectors->list[0], vectors->list[n],
                                      vectors->list[m]},
                    *face_counter_out * 3, 3);
        cpy_uint_al(indices_out->n_indices,
                    (unsigned int[3]){normals->list[0], normals->list[n],
                                      normals->list[m]},
                    *face_counter_out * 3, 3);
        cpy_uint_al(indices_out->t_indices,
                    (unsigned int[3]){tex_coords->list[0], tex_coords->list[n],
                                      tex_coords->list[m]},
                    *face_counter_out * 3, 3);

        (*face_counter_out)++;
    }

    destroy_uint_al(vectors);
    destroy_uint_al(normals);
    destroy_uint_al(tex_coords);
}

bool load_model(const char *path, const char *filename, model_t *model) {
    FILE *fp;
    char *filepath = get_filepath(path, filename);
    fp = fopen(filepath, "r+");
    if (!fp) {
        printf("Error opening file: %s\n", filepath);
        free(filepath);
        return false;
    }
    free(filepath);

    unsigned int face_counter = 0;

    vec3_arraylist_t *vertices;
    vec3_arraylist_t *tex_coords;
    vec3_arraylist_t *normals;
    vec3_arraylist_t *vec3_als = malloc(sizeof(*vertices) * 3);
    if (vec3_als == NULL) {
        printf("Error allocating vec3 arrays\n");
        return false;
    }

    vertices = &vec3_als[0];
    tex_coords = &vec3_als[1];
    normals = &vec3_als[2];

    init_vec3_al(vertices, VEC_START_SIZE);
    init_vec3_al(tex_coords, VEC_START_SIZE);
    init_vec3_al(normals, VEC_START_SIZE);

    /* bool uses_t = false; */
    /* bool uses_n = false; */

    // Do this for every mesh?
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

    mesh_arraylist_t *meshes = malloc(sizeof(*meshes));
    init_mesh_al(meshes, MESH_START_SIZE);

    mtl_arraylist_t *mtls = malloc(sizeof(*mtls));
    init_mtl_al(mtls, MTL_START_SIZE);

    tex_arraylist_t *texs = malloc(sizeof(*texs));
    init_tex_al(texs, TEX_START_SIZE);

    char line[BUFFER_SIZE];

    mesh_t *current_mesh = malloc(sizeof(*current_mesh));

    /* char *line; */
    while (fgets(line, BUFFER_SIZE, fp) != NULL) {
        LINE_CODE code = get_line_code(line);
        switch (code) {
        case NULL_CODE:
        case EMPTY:
        case COMMENT:
            break;
        case MTL_LIB:
            if (!load_mtls(mtls, texs, &line[7], path)) return false;
            break;
        case USE_MTL:
            if (current_mesh->mtl) {
                set_mesh_indices(current_mesh, &indices);
                current_mesh->triangle_count = face_counter;
                append_mesh_al(meshes, current_mesh);
                reset_indices(&indices);
                face_counter = 0;
            }
            set_mtl(current_mesh, mtls->list, mtls->span, &line[7]);
            break;
        case OBJECT:
        case GROUP:
            // TODO: OBJECT and GROUP tags
            break;
        case VERTEX:
            load_v(vertices, &line[2]);
            break;
        case VERTEX_NORMAL:
            load_v(normals, &line[3]);
            break;
        case VERTEX_TEX:
            load_v(tex_coords, &line[3]);
            break;
        case FACE:
            load_face(&indices, &face_counter, &line[2]);
            break;
        default:
            break;
        }
    }

    if (face_counter != 0) {
        set_mesh_indices(current_mesh, &indices);
        current_mesh->triangle_count = face_counter;
        append_mesh_al(meshes, current_mesh);
        reset_indices(&indices);
    }

    model->mesh_count = meshes->span;
    model->meshes = malloc(sizeof(*model->meshes) * meshes->span);
    memcpy(model->meshes, meshes->list, sizeof(*model->meshes) * meshes->span);

    model->vertex_count = vertices->span;
    model->vertices = malloc(sizeof(*model->vertices) * vertices->span);
    memcpy(model->vertices, vertices->list,
           sizeof(*model->vertices) * vertices->span);

    if (normals->span) {
        model->normals = malloc(sizeof(*model->normals) * normals->span);
        memcpy(model->normals, normals->list,
               sizeof(*model->normals) * normals->span);
    }

    if (tex_coords->span) {
        model->tex_coords =
            malloc(sizeof(*model->tex_coords) * tex_coords->span);
        memcpy(model->tex_coords, tex_coords->list,
               sizeof(*model->tex_coords) * tex_coords->span);
    }

    if (texs->span) {
        model->textures = malloc(sizeof(*model->textures) * texs->span);
        memcpy(model->textures, texs->list,
               sizeof(*model->textures) * texs->span);
    }

    free(vec3_als);

    free(indices_);
    free(meshes);
    free(mtls);
    free(texs);

    free(current_mesh);
    fclose(fp);

    return true;
}
