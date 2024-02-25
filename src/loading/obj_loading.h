#ifndef OBJ_LOADING_H
#define OBJ_LOADING_H

#include "../engine.h"

bool load_model(const char* filename, model_t *model);
bool load_mesh(const char* filename, mesh_t *mesh);

#endif

