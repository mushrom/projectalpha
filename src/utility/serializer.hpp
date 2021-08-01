#pragma once

#include <grend/ecs/ecs.hpp>

using namespace grendx;
using namespace grendx::ecs;

// TODO: should this be moved to ecs, utility?
template <class T>
static inline nlohmann::json setSerializedPosition(glm::vec3 position) {
	nlohmann::json asdf = T::defaultProperties();
	asdf["node"]["position"] = { position[0], position[1], position[2] };
	return asdf;
}
