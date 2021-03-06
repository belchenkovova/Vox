#pragma once

#include "application/common/imports/glm.h"

namespace				game
{
    class				random_noise_1d;
}

class 					game::random_noise_1d
{
public :

                        random_noise_1d(float seed) : seed(1.f + mod((seed / 100000.f), 100.f)) {}
                        ~random_noise_1d() = default;

    float				operator () (vec2 input) const
    {
        input = mod(input, vec2(10000.f));
        return ((float)fract(sin(dot(input, const_vector_1d_v2)) * (const_factor * seed)));
    }

	float				operator () (vec3 input) const
	{
		input = mod(input, vec3(10000.f));
		return ((float)fract(sin(dot(input, const_vector_1d_v3)) * (const_factor * seed)));
	}

private :

	const float 		seed;

    static inline float	const_factor = 43758.5453f;
    static inline vec2	const_vector_1d_v2 = vec2(12.9898f, 78.233f);
	static inline vec3	const_vector_1d_v3 = vec3(12.9898f, 78.233f, 12.9898f);
};