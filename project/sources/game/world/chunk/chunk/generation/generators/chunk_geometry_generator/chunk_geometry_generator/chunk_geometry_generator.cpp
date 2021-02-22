#include "chunk_geometry_generator.h"

#include "game/world/chunk/texture_atlas/texture_atlas/texture_atlas.h"
#include "game/world/chunk/block/block/block_settings.h"
#include "game/world/chunk/block/block/block.h"
#include "game/world/chunk/block/block_pointer/block_pointer.h"
#include "game/world/chunk/chunk/generation/chunk_workspace/chunk_workspace.h"

#include "game/world/chunk/chunk/generation/generators/chunk_geometry_generator/data/vertices.h"
#include "game/world/chunk/chunk/generation/generators/chunk_geometry_generator/data/texture_coordinates.h"
#include "game/world/chunk/chunk/generation/generators/chunk_geometry_generator/data/indices.h"
#include "game/world/chunk/chunk/generation/generators/chunk_geometry_generator/tools/vector_tools/vector_tools.h"
#include "game/world/chunk/chunk/generation/generators/chunk_geometry_generator/tools/ao_calculator/ao_calculator.h"

using namespace		game;

void				chunk_geometry_generator::process(const shared_ptr<chunk_workspace> &workspace)
{
	debug::check_critical
	(
		workspace->state == chunk_workspace::light_done,
		"[chunk_geometry_builder] Chunk workspace has unexpected state"
	);

	workspace->state = chunk_workspace::geometry_in_process;

	workspace->batch_for_opaque.filter = &chunk_geometry_generator::filter_for_opaque;
	workspace->batch_for_opaque.geometry_future = async
	(
		launch::async,
		process_batch,
		workspace,
		ref(workspace->batch_for_opaque)
	);

	workspace->batch_for_transparent.filter = &chunk_geometry_generator::filter_for_transparent;
	workspace->batch_for_transparent.geometry_future = async
	(
		launch::async,
		process_batch,
		workspace,
		ref(workspace->batch_for_transparent)
	);

	workspace->batch_for_partially_transparent.filter = &chunk_geometry_generator::filter_for_partially_transparent;
	workspace->batch_for_partially_transparent.geometry_future = async
	(
		launch::async,
		process_batch,
		workspace,
		ref(workspace->batch_for_partially_transparent)
	);
}

void				chunk_geometry_generator::wait(const shared_ptr<chunk_workspace> &workspace)
{
	if
	(
		workspace->batch_for_opaque.geometry_future->wait_for(chrono::seconds(0)) == future_status::ready &&
		workspace->batch_for_transparent.geometry_future->wait_for(chrono::seconds(0)) == future_status::ready &&
		workspace->batch_for_partially_transparent.geometry_future->wait_for(chrono::seconds(0)) == future_status::ready
	)
	{
		workspace->state = chunk_workspace::geometry_done;

	}
}

bool				chunk_geometry_generator::filter_for_opaque(const block &block)
{
	return is_opaque(get_meta_type(block.get_type()));
}

bool				chunk_geometry_generator::filter_for_transparent(const block &block)
{
	return is_transparent(get_meta_type(block.get_type()));
}

bool				chunk_geometry_generator::filter_for_partially_transparent(const block &block)
{
	return is_partially_transparent(get_meta_type(block.get_type()));
}

void				chunk_geometry_generator::process_batch
					(
						shared_ptr<chunk_workspace> workspace,
						chunk_workspace::batch &batch
					)
{
	for (auto &iterator : *workspace->chunk)
	{
		if (batch.filter(iterator->get_value()))
			process_block(workspace, batch, iterator.get_index());
	}
}

void				chunk_geometry_generator::process_block
					(
						const shared_ptr<chunk_workspace> &workspace,
						chunk_workspace::batch &batch,
						const chunk::index &index
					)
{
	const auto 		this_block = block_pointer(workspace->chunk, index);

	if (is_empty(get_meta_type((this_block->get_type()))))
		return ;

	if (is_diagonal(get_meta_type((this_block->get_type()))))
	{
		generate_quad(batch, this_block, block_face::left, this_block->get_light_level());
		generate_quad(batch, this_block, block_face::right, this_block->get_light_level());
	}
	else
	{
		for (block_face face : get_all_block_faces())
		{
			if (auto neighbor_block = this_block.get_neighbor(face); neighbor_block.is_valid())
			{
				if (should_generate_quad(batch, this_block, neighbor_block))
					generate_quad(batch, this_block, face, neighbor_block->get_light_level());
			}
			else
			{
				// If there is no neighbor block, therefore this block is end of world, so we need to draw it
				generate_quad(batch, this_block, face, block_settings::default_light_level);
			}
		}
	}
}

bool				chunk_geometry_generator::should_generate_quad
					(
						chunk_workspace::batch &batch,
						const block_pointer &this_block_pointer,
						const block_pointer &neighbor_block_pointer
					)
{

	const auto		&this_block = *this_block_pointer;
	const auto		&neighbor_block = *neighbor_block_pointer;

	const auto		this_block_meta_type = get_meta_type(this_block.get_type());
	const auto		neighbor_block_meta_type = get_meta_type(neighbor_block.get_type());

	if (is_opaque(this_block_meta_type) and is_transparent_or_partially_transparent(neighbor_block_meta_type));
	else if (is_transparent(this_block_meta_type) and is_partially_transparent(neighbor_block_meta_type));
	else if (is_partially_transparent(this_block_meta_type) and is_partially_transparent(neighbor_block_meta_type));
	else if (is_empty(neighbor_block_meta_type));
	else
		return false;

	return true;
}

void 				chunk_geometry_generator::generate_quad
					(
						chunk_workspace::batch &batch,
						const block_pointer &block,
						block_face face,
						float light_level
					)
{
	generate_indices(batch);
	generate_vertices(batch, block, face);
	generate_texture_coordinates(batch, block, face);
	generate_light_levels(batch, block, face, light_level);
}

void				chunk_geometry_generator::generate_indices(chunk_workspace::batch &batch)
{
	const int		offset = (int)batch.indices.size() / 6 * 4;

	vector_tools::append(batch.indices, indices);
	for (int i = (int)batch.indices.size() - 6; i < (int)batch.indices.size(); i++)
		batch.indices[i] += offset;
}

void				chunk_geometry_generator::generate_vertices
					(
						chunk_workspace::batch &batch,
						const block_pointer &block,
						block_face face
					)
{
	const auto		block_type = block->get_type();
	const auto		block_meta_type = get_meta_type(block_type);

	if (face == block_face::right)
	{
		if (is_diagonal(block_meta_type))
		{
			vector_tools::append(batch.vertices, first_diagonal_vertices);
			vector_tools::append(batch.texture_coordinates, first_diagonal_texture_coordinates);
		}
		else
		{
			vector_tools::append(batch.vertices, right_vertices);
			vector_tools::append(batch.texture_coordinates, right_texture_coordinates);
		}
	}
	else if (face == block_face::left)
	{
		if (is_diagonal(block_meta_type))
		{
			vector_tools::append(batch.vertices, second_diagonal_vertices);
			vector_tools::append(batch.texture_coordinates, second_diagonal_texture_coordinates);
		}
		else
		{
			vector_tools::append(batch.vertices, left_vertices);
			vector_tools::append(batch.texture_coordinates, left_texture_coordinates);
		}
	}
	else if (face == block_face::top)
	{
		vector_tools::append(batch.vertices, top_vertices);
		vector_tools::append(batch.texture_coordinates, top_texture_coordinates);
	}
	else if (face == block_face::bottom)
	{
		vector_tools::append(batch.vertices, bottom_vertices);
		vector_tools::append(batch.texture_coordinates, bottom_texture_coordinates);
	}
	else if (face == block_face::front)
	{
		vector_tools::append(batch.vertices, front_vertices);
		vector_tools::append(batch.texture_coordinates, front_texture_coordinates);
	}
	else if (face == block_face::back)
	{
		vector_tools::append(batch.vertices, back_vertices);
		vector_tools::append(batch.texture_coordinates, back_texture_coordinates);
	}
	else
		debug::raise_error("[chunk_geometry_builder] Can't generate vertices");

	for (int i = (int)batch.vertices.size() - 12; i < (int)batch.vertices.size(); i += 3)
	{
		batch.vertices[i + 0] += (float)block.get_index().x;
		batch.vertices[i + 1] += (float)block.get_index().y;
		batch.vertices[i + 2] += (float)block.get_index().z;
	}

}

void				chunk_geometry_generator::generate_texture_coordinates
					(
						chunk_workspace::batch &batch,
						const block_pointer &block,
						block_face face
					)
{
	static const
	auto			transform_texture_coordinate = [](const ivec2 &texture_coordinates, float &x, float &y)
	{
		static const
		vec2		size = texture_atlas::get_texture_size();

		x = size.x * ((float)texture_coordinates.x + x);
		y = size.y * ((float)texture_coordinates.y + y);
	};

	auto					texture_coordinates = ivec2(0);

	switch (face)
	{
		case block_face::right:
			texture_coordinates = texture_atlas::get_coordinates(block->get_type()).get_right();
			break;

		case block_face::left:
			texture_coordinates = texture_atlas::get_coordinates(block->get_type()).get_left();
			break;

		case block_face::top:
			texture_coordinates = texture_atlas::get_coordinates(block->get_type()).get_top();
			break;

		case block_face::bottom:
			texture_coordinates = texture_atlas::get_coordinates(block->get_type()).get_bottom();
			break;

		case block_face::front:
			texture_coordinates = texture_atlas::get_coordinates(block->get_type()).get_front();
			break;

		case block_face::back:
			texture_coordinates = texture_atlas::get_coordinates(block->get_type()).get_back();
			break;

		default :
			debug::raise_error("[chunk_geometry_builder] Can't generate texture coordinates");
	}

	for (int i = (int)batch.texture_coordinates.size() - 8; i < (int)batch.texture_coordinates.size(); i += 2)
		transform_texture_coordinate(texture_coordinates, batch.texture_coordinates[i + 0], batch.texture_coordinates[i + 1]);
}

void 				chunk_geometry_generator::generate_light_levels
					(
						chunk_workspace::batch &batch,
						const block_pointer &block,
						block_face face,
						float light_level
					)
{
	static const
	auto			combine_light_and_ao = [](float light_level, float ao_level)
	{
		constexpr
		auto		ao_weight = 0.4f;

		return light_level - ao_level * light_level * ao_weight;
	};

	light_level = clamp(light_level, block_settings::min_light_level, block_settings::max_light_level);

	for (float &ao : ao_calculator::calculate(block, face))
		batch.light_levels.push_back(combine_light_and_ao(light_level, ao));
}