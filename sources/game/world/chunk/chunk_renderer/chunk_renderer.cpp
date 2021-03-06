#include "chunk_renderer.h"

#include "engine/main/processor/processor_settings.h"
#include "engine/main/rendering/model/model/model.h"
#include "engine/main/rendering/camera/camera/camera.h"
#include "engine/main/rendering/program/program/program.h"

#include "game/world/texture_atlas/texture_atlas/texture_atlas.h"
#include "game/world/chunk/chunk/chunk.h"
#include "game/world/world/world_settings.h"

using namespace			engine;
using namespace			game;

						chunk_renderer::chunk_renderer()
{
	set_layout("System");

	apply_water_tint = false;

	program = make_unique<class program>(path_to_vertex_shader, path_to_fragment_shader);
	uniform_projection = program->create_uniform<mat4>("uniform_projection");
	uniform_view = program->create_uniform<mat4>("uniform_view");
	uniform_transformation = program->create_uniform<mat4>("uniform_transformation");
	uniform_texture = program->create_uniform<int>("uniform_texture");
	uniform_alpha_discard_floor = program->create_uniform<float>("uniform_alpha_discard_floor");
	uniform_background = program->create_uniform<vec3>("uniform_background");
	uniform_fog_density = program->create_uniform<float>("uniform_fog_density");
	uniform_fog_gradient = program->create_uniform<float>("uniform_fog_gradient");
	uniform_apply_water_tint = program->create_uniform<int>("uniform_apply_water_tint");

	program->use(true);
	uniform_background.upload(processor_settings::background);
	uniform_texture.upload(0);
	uniform_fog_gradient.upload(15.f);
	uniform_apply_water_tint.upload(0);
	program->use(false);
}

void					chunk_renderer::set_apply_water_tint(bool value)
{
	get_instance()->apply_water_tint = value;
}

void					chunk_renderer::render(const shared_ptr<chunk> &chunk, group group)
{
	if (not chunk->is_valid() or not chunk->is_visible)
		return ;

	switch (group)
	{
		case (group::opaque) :
			render(chunk->model_for_opaque);
			break ;

		case (group::transparent) :
			render(chunk->model_for_transparent);
			break ;

		case (group::partially_transparent) :
			render(chunk->model_for_partially_transparent, 0.8f);
			break ;
	}
}

void					chunk_renderer::render
						(
							const shared_ptr<engine::model> &model,
							float alpha_discard_floor
						)
{
	const auto 			instance = get_instance();

	if (!debug::check(model != nullptr, "[game::chunk_renderer] Model is nullptr"))
		return;

	instance->program->use(true);

	instance->uniform_projection.upload(camera::get_instance()->get_projection_matrix());
	instance->uniform_view.upload(camera::get_instance()->get_view_matrix());
	instance->uniform_fog_density.upload(1.f / (world_settings::fog_limit[world_settings::current_visibility_option]));
	instance->uniform_alpha_discard_floor.upload(alpha_discard_floor);
	instance->uniform_apply_water_tint.upload(get_instance()->apply_water_tint);

	model->use(true);
	texture_atlas::use(true);

	instance->uniform_transformation.upload(model->get_transformation());
	model->render();

	texture_atlas::use(false);
	model->use(false);
	instance->program->use(false);
}