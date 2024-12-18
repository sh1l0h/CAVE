#ifndef CAVE_NOISE_H
#define CAVE_NOISE_H

#include "util.h"

typedef struct Noise {
    u32 seed;
    i32 p[512];
} Noise;

void noise_create(Noise *noise, u32 seed);

f32 noise_2d_perlin(Noise *noise, const Vec2 *pos);
f32 noise_3d_perlin(Noise *noise, const Vec3 *pos);

f32 noise_2d_octave_perlin(Noise *noise, const Vec2 *pos, u32 octaves,
                           f32 persistence);
f32 noise_3d_octave_perlin(Noise *noise, const Vec3 *pos, u32 octaves,
                           f32 persistence);

f32 noise_2d_ridged_perlin(Noise *noise, const Vec2 *pos, u32 octaves,
                           f32 offset, f32 lacunarity, f32 persistence);
f32 noise_3d_ridged_perlin(Noise *noise, const Vec3 *pos, u32 octaves,
                           f32 offset, f32 lacunarity, f32 persistence);

#endif
