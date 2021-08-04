#pragma once

#include <grend/ecs/ecs.hpp>
#include <grend/ecs/message.hpp>

#include <memory>

using namespace grendx;
using namespace grendx::ecs;

// I guess use uppercase first letter to indicate
// that this is a singleton
messaging::endpoint* Messages(void);
void ClearMessages(void);
