#pragma once

namespace							game
{
	struct							world_settings;
}

struct								game::world_settings
{
	static inline array<float, 3>	fog_limit = { 150.f, 300.f, 450.f };
	static inline array<float, 3>	visibility_limit = { 250.f, 400.f, 550.f };
	static inline array<float, 3>	cashing_limit = { 250.f, 400.f, 550.f };

	static inline int 				current_visibility_option = 1;
	static inline int 				max_visibility_option = 2;

	// TODO Delete this?
	static inline float				chunks_generation_time_limit = 1.f / 1000.f;
};