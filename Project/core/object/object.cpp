#include "object.h"

#include "core/core/core.h"

void				object::connect_to_core()
{
	core::instance()->new_objects.push_back(shared_from_this());
}