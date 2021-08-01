#pragma once

#include <grend/gameObject.hpp>
#include <grend/animation.hpp>
#include <grend/gameEditor.hpp>
#include <grend/ecs/ecs.hpp>
#include <grend/ecs/collision.hpp>
#include <grend/ecs/rigidBody.hpp>

#include <components/health.hpp>
#include <components/itemPickup.hpp>

#include <components/actions/Wieldable.hpp>
#include <components/actions/Throwable.hpp>

using namespace grendx;
using namespace grendx::ecs;

// TODO: should this be moved to ecs, utility?
template <class T>
static inline nlohmann::json setSerializedPosition(glm::vec3 position) {
	nlohmann::json asdf = T::defaultProperties();
	asdf["node"]["position"] = { position[0], position[1], position[2] };
	return asdf;
}

#include <entities/items/chestItem.hpp>
#include <entities/items/coinPickup.hpp>
#include <entities/items/flareItem.hpp>
#include <entities/items/healthPickup.hpp>
