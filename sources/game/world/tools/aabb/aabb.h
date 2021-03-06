#pragma once

#include "application/common/imports/glm.h"
#include "application/common/imports/std.h"

namespace			game
{
	class			aabb;
}

class 				game::aabb
{
public :
					aabb(const vec3 &min, const vec3 &max) : min(min), max(max) {}
					~aabb() = default;

	static bool		do_collide(const aabb &first, const aabb &second)
	{
		return (
			first.min.x <= second.max.x and first.max.x >= second.min.x
			and first.min.y <= second.max.y and first.max.y >= second.min.y
			and first.min.z <= second.max.z and first.max.z >= second.min.z);
	}

	const vec3		min;
	const vec3		max;
};