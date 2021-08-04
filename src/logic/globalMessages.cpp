#include <grend/ecs/ecs.hpp>
#include <grend/ecs/message.hpp>

#include <memory>

using namespace grendx;
using namespace grendx::ecs;

// bit of a hack, avoids creating another c++ source file...
static std::unique_ptr<messaging::endpoint> globalMessages
	= std::make_unique<messaging::endpoint>();

messaging::endpoint* Messages() { return globalMessages.get(); }

void ClearMessages(void) {
	// XXX, maybe
	globalMessages = std::make_unique<messaging::endpoint>();
}
