#include <entities/items/items.hpp>
#include <components/actions/action.hpp>
#include <components/actions/Throwable.hpp>
#include <components/actions/Wieldable.hpp>
#include <components/removeBodiesOnWorldCollision.hpp>

// non-pure destructors for rtti
healthPickup::~healthPickup() {};
healthPickupCollision::~healthPickupCollision() {};
coinPickup::~coinPickup() {};
chestItem::~chestItem() {};
flareItem::~flareItem() {};
explodyBarrel::~explodyBarrel() {};
