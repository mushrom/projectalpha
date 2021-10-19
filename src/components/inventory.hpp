#pragma once

#include <grend/gameObject.hpp>
#include <grend/ecs/ecs.hpp>

using namespace grendx;
using namespace grendx::ecs;

class inventory : public component {
	public:
		constexpr static const char *serializedType = "inventory";

		inventory(entityManager *manager, entity *ent, nlohmann::json properties)
			: component(manager, ent)
		{
			manager->registerComponent(ent, serializedType, this);
		}

		virtual ~inventory();
		virtual const char* typeString(void) const { return serializedType; };

		void insert(entityManager *manager, entity *ent);
		entity *remove(entityManager *manager, entity *ent);
		entity *findType(const std::string& type);

		std::map<std::string, std::set<entity*>> items;
};
