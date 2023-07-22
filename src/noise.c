#include "include/noise.h"

static void swap(i32 *arr, i32 i, i32 j)
{
	i32 tmp = arr[i];
	arr[i] = arr[j];
	arr[j] = tmp;
}

static f32 lerp(f32 a, f32 b, f32 t)
{
	return a + t*(b - a);
}

static f32 fade(f32 t)
{
	return ((6.0f*t-15.0f)*t+10.0f)*t*t*t;
}

static f32 grad_2d(u8 hash, f32 x, f32 y)
{
	switch(hash & 3){
	case 0:
		return x + y;
	case 1:
		return -x + y;
	case 2:
		return -x - y;
	case 3:
		return x - y;
	}
	return 0.0f;
}

void noise_create(Noise *noise, u32 seed)
{
	noise->seed = seed;

	i32 per[256];

	for(i32 i = 0; i < 256; i++){
		per[i] = i;
	}

	srand(seed);
	for(i32 i = 255; i >= 0; i--){
		i32 j = rand() % (i + 1);
		swap(per, i, j);
	}

	for(i32 i = 0; i < 256; i++){
		noise->p[256 +i] = noise->p[i] = per[i];
	}
}

f32 noise_2d_perlin(Noise *noise, const Vec2 *pos)
{
	i32 x = (u32)floorf(pos->x) & 255;
	i32 y = (u32)floorf(pos->y) & 255;

	f32 xf = pos->x - floorf(pos->x);
	f32 yf = pos->y - floorf(pos->y);

	f32 u = fade(xf);
	f32 v = fade(yf);

	i32 *P = noise->p;
	i32 A = P[x] + y, B = P[x+1] + y;

	return lerp(lerp(grad_2d(P[A    ], xf       , yf       ),
					 grad_2d(P[A + 1], xf       , yf - 1.0f), v),
				lerp(grad_2d(P[B    ], xf - 1.0f, yf       ),
					 grad_2d(P[B + 1], xf - 1.0f, yf - 1.0f), v), u);
}

//void noise_3d_prelin(Noise *noise, Vec3 *pos)
//{
//}
//

f32 noise_2d_octave_perlin(Noise *noise, const Vec2 *pos, u32 octaves, f32 persistence)
{
	f32 sum = 0.0f;
	f32 freq = 1.0f;
	f32 amp = 1.0f;
	f32 amp_sum = 0.0f;

	Vec2 tmp;
	zinc_vec2_copy(pos, &tmp);

	for(u32 i = 0; i < octaves; i++){
		sum += noise_2d_perlin(noise, &tmp)*amp;
		amp_sum += amp;

		amp *= persistence;
		freq *= 2;
		zinc_vec2_scale(&tmp, freq, &tmp);
	}

    if (sum > 1.0f) sum = 1.0f;
    else if (sum < -1.0f) sum = -1.0f;

	return sum;
}

//void noise_3d_octave_perlin(Noise *noise, Vec3 *pos, u32 octaves, u32 persistence)
//{
//}
