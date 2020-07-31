#include "loader.h"

						loader::loader() : file(loader_settings::path_to_profile)
{
	file.open();
	if (file.is_empty())
		file << loader_settings::header;
	else
	{
		file::string	test(loader_settings::header.size());

		file >> test;
		assert((string)test == loader_settings::header and "Profile has invalid format");
	}
}

						loader::~loader()
{
	file.close();
}

void					loader::initializer(shared_ptr<loader> loader)
{
	static_assert(sizeof(block::type) == 1u and "Block type should be size of byte");
	chunk_loader::pointer = static_pointer_cast<chunk_loader>(loader);
}

shared_ptr<chunk>		loader::download_implementation(const vec3 &position)
{
	int					pointer;
	bool 				have_found_chunk = false;

	chunk_state			test_state;
	vec3				test_position;

	file.read_pointer = loader_settings::header.size();
	file.write_pointer = loader_settings::header.size();

	while (not file.is_eof())
	{
		pointer = file.read_pointer;

		file >> (char &)test_state;
		file >> test_position;

		if (test_position == position)
		{
			have_found_chunk = true;
			break ;
		}

		file >> file::ignore(loader_settings::chunk_linear_size);
	}

	assert(have_found_chunk and "Chunk not found");

	shared_ptr<chunk>	chunk;

	chunk = chunk::create(position);
	chunk_editor::use(chunk);
	for (auto iterator = chunk_editor::begin(); iterator != chunk_editor::end(); iterator++)
		file >> (char &)chunk_editor::at(iterator);

	file.write_pointer = pointer;
	file << (char)(chunk_state::invalid);

	return (chunk);
}

void					loader::upload_implementation(const shared_ptr<chunk> &chunk)
{
	bool 				have_found_place_for_writing = false;
	int					pointer = 0;

	chunk_state			state;

	file.read_pointer = loader_settings::header.size();
	file.write_pointer = loader_settings::header.size();

	while (true)
	{
		pointer = file.read_pointer;

		if (not (file >> (char &)state))
			break ;

		file >> file::ignore<float>(3);
		file >> file::ignore(loader_settings::chunk_linear_size);

		if (state != chunk_state::valid)
		{
			have_found_place_for_writing = true;
			break;
		}
	}

	file.clear();

	if (have_found_place_for_writing)
		file.write_pointer = pointer;
	else
		file.write_pointer = file::label::end;

	file << (char)chunk_state::valid;
	file << chunk->get_position();

	chunk_editor::use(chunk);
	for (auto iterator = chunk_editor::begin(); iterator != chunk_editor::end(); iterator++)
		file << (char)chunk_editor::at(iterator);
}