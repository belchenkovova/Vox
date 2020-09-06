#pragma once

#include "common/aliases.h"
#include "common/property.h"
#include "world/block/block/block.h"
#include "world/generator/noise/perlin_noise.h"
#include "world/generator/noise/cellular_noise.h"

namespace					world
{
	class 					biome;
}

class 						world::biome
{
public :

	enum					type
	{
		null,
		test_dirt,
		test_stone
	};
							property<read_only, enum type, biome>
							type;

							property<read_only, enum block::type, biome>
							first_layer;

	explicit				biome(enum type type = biome::null)
	{
		switch (this->type.value = type)
		{
			case (test_dirt) :
				noise = cellular_noise(1.f, 0.01f, 30.f);
				first_layer = block::dirt;
				break ;

			case (test_stone) :
				noise = cellular_noise(1.f, 0.01f, 30.f);
				first_layer = block::stone;
				break ;

			default :
				break ;
		}

		first_layer.getter = [this]()
		{
			assert(this->type != biome::null);
			return (first_layer.value);
		};
	}

						biome(const biome &other)
	{
		type = other.type;
		first_layer = other.first_layer.value;
		noise = other.noise;
	}

	int 					height(const vec3 &position) const
	{
		assert(type != biome::null);
		return (noise.generate(vec2(position.x, position.z)).final_distance * 15);
	}

	friend bool 			operator == (const biome &left, const biome &right)
	{
		return (left.type == right.type);
	}

	friend bool 			operator != (const biome &left, const biome &right)
	{
		return (left.type != right.type);
	}

private :

	cellular_noise			noise;
};