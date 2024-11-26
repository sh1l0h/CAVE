#ifndef CAVE_COMPONENTS_H
#define CAVE_COMPONENTS_H

#include "../util.h"
#include "../world/chunk.h"

#define COMPONENTS \
    X(Transform)   \
    X(Camera)      \
    X(Player)      \
    X(BoxCollider) \
    X(RigidBody)

typedef enum ComponentID {
#define X(_name) CMP_##_name,
    COMPONENTS
#undef X
    CMP_COUNT // Always keep this as the last entry
} ComponentID;

extern u32 component_sizes[CMP_COUNT];

i32 cmp_id_cmp(const void *a, const void *b);

//COMPONENTS

typedef struct Transform {
    Vec3 position;
    Vec3 rotation;

    Vec3 forward;
    Vec3 up;
    Vec3 right;
} Transform;

typedef struct Camera {
    f32 fov, aspect_ratio, near, far;
    Mat4 view, projection;
} Camera;

typedef struct Player {
    Vec3 acceleration;
    bool is_sneaking;

    Vec3i chunk_pos;
    Chunk *selected_block_chunk;
    Vec3i selected_block_offset;
    Direction selected_block_dir;
} Player;

typedef struct BoxCollider {
    Vec3 half_size;
    Vec3 offset;
} BoxCollider;

typedef struct RigidBody {
    Vec3 velocity;

    bool on_ground;
    bool gravity;
} RigidBody;

#endif
