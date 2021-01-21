#include "block_selector.h"

#include "engine/main/rendering/model/model/model.h"

#include "game/world/chunk/block/block_selector/block_selector_renderer/block_selector_renderer.h"

using namespace			engine;
using namespace			game;

static vector<float>	cube_vertices =
{
	-0.5f, -0.5f, -0.5f,
	-0.5f, +0.5f, -0.5f,
	+0.5f, +0.5f, -0.5f,
	+0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, +0.5f,
	-0.5f, +0.5f, +0.5f,
	+0.5f, +0.5f, +0.5f,
	+0.5f, -0.5f, +0.5f,
};

static vector<GLuint>	cube_indices =
{
	0, 1,
	1, 2,
	2, 3,
	3, 0,

	4, 5,
	5, 6,
	6, 7,
	7, 4,

	0, 4,
	1, 5,
	2, 6,
	3, 7
};

static vector<float>	left_vertices =
{
	-0.5f, +0.5f, -0.5f,
	-0.5f, -0.5f, +0.5f,
	-0.5f, +0.5f, +0.5f,
	-0.5f, -0.5f, -0.5f
};

static vector<float>	right_vertices =
{
	+0.5f, +0.5f, -0.5f,
	+0.5f, -0.5f, +0.5f,
	+0.5f, +0.5f, +0.5f,
	+0.5f, -0.5f, -0.5f
};

static vector<float>	front_vertices =
{
	-0.5f, +0.5f, +0.5f,
	+0.5f, -0.5f, +0.5f,
	+0.5f, +0.5f, +0.5f,
	-0.5f, -0.5f, +0.5f
};

static vector<float>	back_vertices =
{
	-0.5f, +0.5f, -0.5f,
	+0.5f, -0.5f, -0.5f,
	+0.5f, +0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f
};

static vector<float>	top_vertices =
{
	-0.5f, +0.5f, -0.5f,
	+0.5f, +0.5f, +0.5f,
	-0.5f, +0.5f, +0.5f,
	+0.5f, +0.5f, -0.5f
};

static vector<float>	bottom_vertices =
{
	-0.5f, -0.5f, -0.5f,
	+0.5f, -0.5f, +0.5f,
	-0.5f, -0.5f, +0.5f,
	+0.5f, -0.5f, -0.5f
};

static vector<GLuint>	face_indices = {0, 1, 2, 3};

						block_selector::block_selector()
{
	set_layout("Transparent");
}

vec3					block_selector::get_translation()
{
	return get_instance()->translation;
}

block::face				block_selector::get_selected_face()
{
	return get_instance()->selected_face;
}

void					block_selector::set_translation(const vec3 &value)
{
	const auto			instance = get_instance();

	instance->translation = value;

	instance->cube->set_translation(value);
	instance->left->set_translation(value);
	instance->right->set_translation(value);
	instance->front->set_translation(value);
	instance->back->set_translation(value);
	instance->top->set_translation(value);
	instance->bottom->set_translation(value);
}

void					block_selector::set_selected_face(block::face face)
{
	get_instance()->selected_face = face;
}

void 					block_selector::when_initialized()
{
	cube = make_shared<::model>(model::lines);
	cube->bind(true);
	cube->add_vbo(3, cube_vertices);
	cube->add_ebo(cube_indices);
	cube->bind(false);

	left = make_shared<::model>(model::lines);
	left->bind(true);
	left->add_vbo(3, left_vertices);
	left->add_ebo(face_indices);
	left->bind(false);

	right = make_shared<::model>(model::lines);
	right->bind(true);
	right->add_vbo(3, right_vertices);
	right->add_ebo(face_indices);
	right->bind(false);

	front = make_shared<::model>(model::lines);
	front->bind(true);
	front->add_vbo(3, front_vertices);
	front->add_ebo(face_indices);
	front->bind(false);

	back = make_shared<::model>(model::lines);
	back->bind(true);
	back->add_vbo(3, back_vertices);
	back->add_ebo(face_indices);
	back->bind(false);

	top = make_shared<::model>(model::lines);
	top->bind(true);
	top->add_vbo(3, top_vertices);
	top->add_ebo(face_indices);
	top->bind(false);

	bottom = make_shared<::model>(model::lines);
	bottom->bind(true);
	bottom->add_vbo(3, bottom_vertices);
	bottom->add_ebo(face_indices);
	bottom->bind(false);
}

void					block_selector::when_rendered()
{
	block_selector_renderer::get_instance()->render(*this);
}