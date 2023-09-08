#include "../../include/core/noise.h"

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

static f32 grad_2d(i32 hash, f32 x, f32 y)
{
	switch(hash & 3){
	case 0: return x + y;
	case 1: return -x + y;
	case 2: return x - y;
	case 3: return -x - y;
	default: return 0; 
	}
	return 0.0f;
}

static f32 grad_3d(int hash, f32 x, f32 y, f32 z)
{
    switch(hash & 0xF)
    {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + z;
        case 0x5: return -x + z;
        case 0x6: return  x - z;
        case 0x7: return -x - z;
        case 0x8: return  y + z;
        case 0x9: return -y + z;
        case 0xA: return  y - z;
        case 0xB: return -y - z;
        case 0xC: return  y + x;
        case 0xD: return -y + z;
        case 0xE: return  y - x;
        case 0xF: return -y - z;
        default: return 0; 
    }
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
	i32 x = (i32)floorf(pos->x) & 255;
	i32 y = (i32)floorf(pos->y) & 255;

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

f32 noise_3d_perlin(Noise *noise, const Vec3 *pos)
{
	i32 x = (i32)floorf(pos->x) & 255;
	i32 y = (i32)floorf(pos->y) & 255;
	i32 z = (i32)floorf(pos->z) & 255;

	f32 xf = pos->x - floorf(pos->x);
	f32 yf = pos->y - floorf(pos->y);
	f32 zf = pos->z - floorf(pos->z);

	f32 u = fade(xf);
	f32 v = fade(yf);
	f32 w = fade(zf);

	i32 *P = noise->p;

	i32 A = P[x  ]+y, AA = P[A]+z, AB = P[A+1]+z,      
		B = P[x+1]+y, BA = P[B]+z, BB = P[B+1]+z;     

	return lerp(lerp(lerp(grad_3d(P[AA  ], xf     , yf     , zf     ),
						  grad_3d(P[BA  ], xf-1.0f, yf     , zf     ), u),
					 lerp(grad_3d(P[AB  ], xf     , yf-1.0f, zf     ),
						  grad_3d(P[BB  ], xf-1.0f, yf-1.0f, zf     ), u), v),
				lerp(lerp(grad_3d(P[AA+1], xf     , yf     , zf-1.0f),
						  grad_3d(P[BA+1], xf-1.0f, yf     , zf-1.0f), u),
					 lerp(grad_3d(P[AB+1], xf     , yf-1.0f, zf-1.0f),
						  grad_3d(P[BB+1], xf-1.0f, yf-1.0f, zf-1.0f), u), v), w);
}


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

f32 noise_3d_octave_perlin(Noise *noise, const Vec3 *pos, u32 octaves, f32 persistence)
{
	f32 sum = 0.0f;
	f32 freq = 1.0f;
	f32 amp = 0.7f;

	Vec3 tmp;
	zinc_vec3_copy(pos, &tmp);

	for(u32 i = 0; i < octaves; i++){
		sum += noise_3d_perlin(noise, &tmp)*amp;

		amp *= persistence;
		freq *= 2;
		zinc_vec3_scale(&tmp, freq, &tmp);
	}

	if (sum > 1.0f) sum = 1.0f;
	else if (sum < -1.0f) sum = -1.0f;

	return sum;
}


f32 noise_2d_ridged_perlin(Noise *noise, const Vec2 *pos, u32 octaves, f32 offset, f32 lacunarity, f32 persistence)
{
	f32 sum = 0.0f;
	f32 freq = 1.0f;
	f32 amp = 0.7f;
	f32 amp_sum = 0.0f;

	Vec2 tmp;
	zinc_vec2_copy(pos, &tmp);

	for(u32 i = 0; i < octaves; i++){
		sum += (offset - fabsf(noise_2d_perlin(noise, &tmp)))*amp;
		amp_sum += offset*amp;
		
		amp *= persistence;
		freq *= lacunarity;
		zinc_vec2_scale(&tmp, freq, &tmp);
	}

	return sum / amp_sum;
}

f32 noise_3d_ridged_perlin(Noise *noise, const Vec3 *pos, u32 octaves, f32 offset, f32 lacunarity, f32 persistence)
{
	f32 sum = 0.0f;
	f32 freq = 1.0f;
	f32 amp = 0.7f;

	f32 amp_sum = 0.0f;

	Vec3 tmp;
	zinc_vec3_copy(pos, &tmp);

	for(u32 i = 0; i < octaves; i++){
		sum += (offset - fabsf(noise_3d_perlin(noise, &tmp)))*amp;
		amp_sum += offset*amp;

		amp *= persistence;
		freq *= lacunarity;
		zinc_vec3_scale(&tmp, freq, &tmp);
	}

	if (sum > 1.0f) sum = 1.0f;
	else if (sum < -1.0f) sum = -1.0f;

	return sum / amp_sum;
}
