#include "player.h"

#include "engine/input/input.h"
#include "world/world/world.h"
#include "player/camera/camera.h"
#include "player/player/player_settings.h"

									player::player()
{
	layout = "system";
	should_be_rendered = false;
}

void								player::update()
{
	process_movement();
	process_interaction();
	process_ray_casting();
}

void 								player::process_movement()
{
	optional<camera::move_request>	request;

	if (input::is_pressed_or_held(GLFW_KEY_A))
		request = camera::move_request::left;
	else if (input::is_pressed_or_held(GLFW_KEY_D))
		request = camera::move_request::right;
//						Axis y
	if (input::is_pressed_or_held(GLFW_KEY_Q))
		request = camera::move_request::down;
	else if (input::is_pressed_or_held(GLFW_KEY_E))
		request = camera::move_request::up;

//						Axis Z
	if (input::is_pressed_or_held(GLFW_KEY_W))
		request = camera::move_request::forward;
	else if (input::is_pressed_or_held(GLFW_KEY_S))
		request = camera::move_request::back;

	if (request)
	{
		const vec3					future_position = camera::peek_position(*request, player_settings::movement_speed);
		const ::aabb				aabb = player::aabb(future_position);

		if (not world::does_collide(aabb))
			camera::move(*request, player_settings::movement_speed);
	}
}

void 								player::process_interaction()
{
	if (input::is_pressed(GLFW_KEY_ENTER))
	{
		if (auto hit = camera::cast_ray(); hit)
		{
			auto 	axis_and_sign = block::to_axis_and_sign(hit->face);
			auto	neighbor = hit->block.neighbor(axis_and_sign.first, axis_and_sign.second);

			assert(neighbor);
			world::insert_block(*neighbor, block::type::dirt_with_grass);

			force_ray_cast = true;
		}
	}
}

void 								player::process_ray_casting()
{
	if (camera::have_changed or force_ray_cast)
	{
		if (auto hit = camera::cast_ray(); hit)
			world::select_block(hit->block, hit->face);
		else
			world::unselect_block();

		force_ray_cast = false;
	}
}

aabb								player::aabb(const vec3 &position) const
{
	return {position - player_settings::aabb_size / 2.f, position +  + player_settings::aabb_size / 2.f};
}