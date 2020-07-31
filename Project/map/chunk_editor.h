#pragma once

#include "map/chunk.h"

class					chunk_editor
{
public :
						chunk_editor() = default;
	virtual				~chunk_editor() = default;

protected :

	void 				use(const shared_ptr<chunk> &chunk)
	{
		this->target = chunk;
	}

	auto				begin() const
	{
		return (target->begin());
	}

	auto				end() const
	{
		return (target->end());
	}

	block::type			&at(chunk::iterator &iterator) const
	{
		return (iterator.value().type_value);
	}

	block::type			&at(chunk::index &index) const
	{
		return (target->at(index).type_value);
	}

private :

	shared_ptr<chunk>	target;
};