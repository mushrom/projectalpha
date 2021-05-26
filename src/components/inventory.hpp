#pragma once

#include <grend/gameObject.hpp>
#include <grend/ecs/ecs.hpp>

using namespace grendx;
using namespace grendx::ecs;

class inventory : public component {
	public:
		inventory(entityManager *manager, entity *ent, nlohmann::json properties)
			: component(manager, ent)
		{
			manager->registerComponent(ent, "inventory", this);
		}

		virtual ~inventory();

		void insert(entityManager *manager, entity *ent);
		entity *remove(entityManager *manager, size_t idx);

		std::vector<entity*> items;
};
