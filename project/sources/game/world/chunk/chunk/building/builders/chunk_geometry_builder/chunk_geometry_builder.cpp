#include "chunk_geometry_builder.h"
#include "vertices.h"
#include "texture_coordinates.h"
#include "indices.h"

#include "game/world/chunk/texture_atlas/texture_atlas/texture_atlas.h"
#include "game/world/chunk/block/block/block_settings.h"
#include "game/world/chunk/block/block/block.h"
#include "game/world/chunk/chunk/building/chunk_workspace/chunk_workspace.h"

using namespace		game;

void				chunk_geometry_builder::launch(const shared_ptr<chunk_workspace> &workspace)
{
	debug::check_critical
	(
		workspace->state == chunk_workspace::light_done,
		"[chunk_geometry_builder] Chunk workspace has unexpected state"
	);

	workspace->state = chunk_workspace::geometry_in_process;

	workspace->batch_for_opaque.filter = &chunk_geometry_builder::filter_for_opaque;
	workspace->batch_for_opaque.geometry_future = async
	(
		launch::async,
		process_batch,
		workspace,
		ref(workspace->batch_for_opaque)
	);

	workspace->batch_for_transparent.filter = &chunk_geometry_builder::filter_for_transparent;
	workspace->batch_for_transparent.geometry_future = async
	(
		launch::async,
		process_batch,
		workspace,
		ref(workspace->batch_for_transparent)
	);

	workspace->batch_for_partially_transparent.filter = &chunk_geometry_builder::filter_for_partially_transparent;
	workspace->batch_for_partially_transparent.geometry_future = async
	(
		launch::async,
		process_batch,
		workspace,
		ref(workspace->batch_for_partially_transparent)
	);
}

void				chunk_geometry_builder::wait(const shared_ptr<chunk_workspace> &workspace)
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

bool				chunk_geometry_builder::filter_for_opaque(const block &block)
{
	return is_opaque(get_meta_type(block.get_type()));
}

bool				chunk_geometry_builder::filter_for_transparent(const block &block)
{
	return is_transparent(get_meta_type(block.get_type()));
}

bool				chunk_geometry_builder::filter_for_partially_transparent(const block &block)
{
	return is_partially_transparent(get_meta_type(block.get_type()));
}

void				chunk_geometry_builder::process_batch
					(
						shared_ptr<chunk_workspace> workspace,
						chunk_workspace::batch &batch
					)
{
	for (auto &iterator : *workspace->chunk)
	{
		if (batch.filter(iterator->value()))
			process_block(workspace, batch, iterator.index());
	}
}

void				chunk_geometry_builder::process_block
					(
						const shared_ptr<chunk_workspace> &workspace,
						chunk_workspace::batch &batch,
						const chunk::index &index
					)
{
	const auto		&this_block = workspace->chunk->at(index);

	if (is_empty(get_meta_type((this_block.get_type()))))
		return ;

	if (is_diagonal(get_meta_type((this_block.get_type()))))
	{
		build_quad(workspace, batch, index, axis::x, sign::minus);
		build_quad(workspace, batch, index, axis::x, sign::plus);
	}
	else
	{
		for (axis axis : {axis::x, axis::y, axis::z})
		for (sign sign : {sign::minus, sign::plus})
		{
			if (should_build_quad(workspace, batch, index, axis, sign))
				build_quad(workspace, batch, index, axis, sign);
		}
	}
}

bool				chunk_geometry_builder::should_build_quad
					(
						const shared_ptr<chunk_workspace> &workspace,
						chunk_workspace::batch &batch,
						const chunk::index &index,
						axis axis,
						sign sign
					)
{
	const auto		this_block_pointer = block_pointer(workspace->chunk, index);
	const auto 		neighbor_block_pointer = this_block_pointer.get_neighbor(axis, sign);

	if (not neighbor_block_pointer) // If there is no neighbor block, therefore this block is end of world, so we need to draw it
		return true;

	const auto		this_block = this_block_pointer();
	const auto		neighbor_block = neighbor_block_pointer.value()();

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

void				chunk_geometry_builder::build_quad
					(
						const shared_ptr<chunk_workspace> &workspace,
						chunk_workspace::batch &batch,
						const chunk::index &index,
						axis axis,
						sign sign
					)
{
	const auto		&block = workspace->chunk->at(index);
	const auto		block_meta_type = get_meta_type(block.get_type());

	char			light_level;
	ivec2			texture_coordinates = ivec2(0);

	light_level = workspace->light_levels.at(index.x, index.y, index.z);
	light_level = max(light_level, block_settings::light_level_min);

	if (axis == axis::x and sign == sign::plus)
	{
		if (is_diagonal(block_meta_type))
		{
			append_to_vector(batch.vertices, first_diagonal_vertices);
			append_to_vector(batch.texture_coordinates, first_diagonal_texture_coordinates);
		}
		else
		{
			append_to_vector(batch.vertices, right_vertices);
			append_to_vector(batch.texture_coordinates, right_texture_coordinates);
		}

		texture_coordinates = texture_atlas::get_coordinates(workspace->chunk->at(index).get_type()).get_right();
	}
	else if (axis == axis::x and sign == sign::minus)
	{
		if (is_diagonal(block_meta_type))
		{
			append_to_vector(batch.vertices, second_diagonal_vertices);
			append_to_vector(batch.texture_coordinates, second_diagonal_texture_coordinates);
		}
		else
		{
			append_to_vector(batch.vertices, left_vertices);
			append_to_vector(batch.texture_coordinates, left_texture_coordinates);
		}

		texture_coordinates = texture_atlas::get_coordinates(workspace->chunk->at(index).get_type()).get_left();
	}
	else if (axis == axis::y and sign == sign::plus)
	{
		append_to_vector(batch.vertices, top_vertices);
		append_to_vector(batch.texture_coordinates, top_texture_coordinates);
		texture_coordinates = texture_atlas::get_coordinates(workspace->chunk->at(index).get_type()).get_top();
	}
	else if (axis == axis::y and sign == sign::minus)
	{
		append_to_vector(batch.vertices, bottom_vertices);
		append_to_vector(batch.texture_coordinates, bottom_texture_coordinates);
		texture_coordinates = texture_atlas::get_coordinates(workspace->chunk->at(index).get_type()).get_bottom();
	}
	else if (axis == axis::z and sign == sign::plus)
	{
		append_to_vector(batch.vertices, front_vertices);
		append_to_vector(batch.texture_coordinates, front_texture_coordinates);
		texture_coordinates = texture_atlas::get_coordinates(workspace->chunk->at(index).get_type()).get_front();
	}
	else if (axis == axis::z and sign == sign::minus)
	{
		append_to_vector(batch.vertices, back_vertices);
		append_to_vector(batch.texture_coordinates, back_texture_coordinates);
		texture_coordinates = texture_atlas::get_coordinates(workspace->chunk->at(index).get_type()).get_back();
	}
	else
		debug::raise_error("[chunk_geometry_builder] Can't build quad");

	for (int i = (int)batch.vertices.size() - 12; i < (int)batch.vertices.size(); i += 3)
	{
		batch.vertices[i + 0] += (float)index.x;
		batch.vertices[i + 1] += (float)index.y;
		batch.vertices[i + 2] += (float)index.z;
	}

	auto					transform_texture_coordinate = [texture_coordinates](float &x, float &y)
	{
		static vec2 		size = texture_atlas::get_texture_size();

		x = size.x * ((float)texture_coordinates.x + x);
		y = size.y * ((float)texture_coordinates.y + y);
	};

	for (int i = (int)batch.texture_coordinates.size() - 8; i < (int)batch.texture_coordinates.size(); i += 2)
		transform_texture_coordinate(batch.texture_coordinates[i + 0], batch.texture_coordinates[i + 1]);

	const int				offset = (int)batch.indices.size() / 6 * 4;

	append_to_vector(batch.indices, indices);
	for (int i = (int)batch.indices.size() - 6; i < (int)batch.indices.size(); i++)
		batch.indices[i] += offset;

	auto 					normalized_light_level = (float)light_level / block_settings::light_level_max;

	for (int i = 0; i < 4; i++)
		batch.light_levels.push_back(normalized_light_level);
}

template					<typename type>
void						chunk_geometry_builder::append_to_vector(vector<type> &target, const vector<type> &source)
{
	target.insert(target.end(), source.begin(), source.end());
}