#pragma once

#include "game/world/chunk/chunk/chunk/chunk_settings.h"
#include "game/world/world/world_settings.h"

namespace					game
{
	struct					camera_settings;
}

// TODO Fix this?
struct						game::camera_settings
{
	static constexpr float	rotation_speed = 0.05f;

	static inline float		near_plane = 0.05f;
	static inline float		far_plane = world_settings::visibility_limit + 100.f;
	static inline float 	fov = 50.f;

	static inline vec3		initial_position = vec3(0.f, 30.0f, 0.f);

	static inline int		ray_cast_limit = 15;
};