#include "core.h"

#include "engine/core/core_settings.h"
#include "engine/window/window.h"
#include "engine/input/input.h"
#include "engine/time/timer/timer.h"

using namespace	engine;

				core::core()
{
	window::initialize();
	input::initialize();
}

void			core::register_layout(const string &key, int options)
{
	auto 		layout = make_shared<::layout>(key, options);

	get_instance()->layouts[key] = layout;
	get_instance()->layouts_order.push_back(layout);
}

void			core::execute()
{
	auto		instance = singleton<core>::get_instance();

	do
	{
		instance->process_input();

		if (window::get_instance()->is_closed())
		{
			for (auto &[name, layout] : instance->layouts)
			for (auto &object : layout->objects)
				object->destroy();
		}

		instance->process_creating();
		instance->process_destroying();
		instance->process_updating();
		instance->process_rendering();
	}
	while (not window::get_instance()->is_closed());
}

void			core::process_input()
{
	static bool	empty_polygons = false;

	input::get_instance()->reset_keys();
	input::get_instance()->update_mouse();

	glfwPollEvents();

	if (input::get_instance()->is_pressed(input::key::escape))
		window::get_instance()->close();
	if (input::get_instance()->is_pressed(input::key::letter_p))
	{
		empty_polygons = not empty_polygons;
		glPolygonMode(GL_FRONT_AND_BACK, empty_polygons ? GL_LINE : GL_FILL);
	}
}

void			core::process_creating()
{
	for (auto &new_object : new_objects)
	{
		if (auto iterator = layouts.find(new_object->layout); iterator != layouts.end())
			iterator->second->objects.push_back(new_object);
		else
			assert(false and "Unknown layout");
	}

	for (auto &layout : layouts_order)
	for (auto &object : layout->objects)
	{
		if (object->state == object::uninitialized)
				object->initialize();
	}

	new_objects.clear();
}

void			core::process_destroying()
{
	for (auto &[name, layout] : layouts)
	for (auto iterator = layout->objects.begin(); iterator != layout->objects.end();)
	{
		if ((*iterator)->should_be_destroyed)
		{
			(*iterator)->deinitialize();
			iterator = layout->objects.erase(iterator);
		}
		else
			++iterator;
	}
}

void			core::process_updating()
{
	for (auto &[name, layout] : layouts)
	for (auto &object : layout->objects)
		if (object->should_be_updated and object->state == object::active)
			object->update();
}

void			core::process_rendering()
{
	auto 		&background = core_settings::background;

	glClearColor(background.x, background.y, background.z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	for (auto &[name, layout] : layouts)
	for (auto &object : layout->objects)
	{
		window::get_instance()->use_depth_test((layout->options & (int)layout::use_depth_test) != 0);
		if (object->should_be_rendered and object->state == object::active)
			object->render();
	}

	window::get_instance()->swap_buffers();
}