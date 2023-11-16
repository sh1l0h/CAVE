#ifndef CAVE_MODEL_H
#define CAVE_MODEL_H

#include "../util.h"
#include "../data_structures/hash_map.h"

typedef struct Face {
	Direction cull_dir;
	u8 positions[12];
	u8 uvs[8];
} Face;

typedef struct Model {
	bool ambient_occlusion;
	Face faces[6];
} Model;

//maps string path to model pointer
extern HashMap models;

i32 models_init(const char *model_dir_path);

Model *models_get(const char *model_path);

#endif
