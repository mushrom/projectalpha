#include <components/actions/action.hpp>
#include <components/actions/Throwable.hpp>
#include <components/actions/Wieldable.hpp>
#include <components/removeBodiesOnWorldCollision.hpp>

// non-pure destructors for rtti
Action::~Action() {};
Consumable::~Consumable() {};
Throwable::~Throwable() {};
healingConsumable::~healingConsumable() {};
Wieldable::~Wieldable() {};
removeBodiesOnWorldCollision::~removeBodiesOnWorldCollision() {};
