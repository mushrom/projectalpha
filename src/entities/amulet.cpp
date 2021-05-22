#include <entities/amulet.hpp>
#include <grend/gameEditor.hpp>

amuletPickup::~amuletPickup() {};

amuletPickup::amuletPickup(entityManager *manager,
                           gameMain *game,
                           glm::vec3 position)
	: entity(manager)
{
	new areaSphere(manager, this, 2.f);
	manager->registerComponent(this, "amuletPickup", this);

	// TODO: resource manager
	static gameObject::ptr amuletModel = nullptr;
	if (!amuletModel) {
		amuletModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/amulet.glb");

		TRS transform = amuletModel->getTransformTRS();
		transform.scale = glm::vec3(2.0);
		amuletModel->setTransform(transform);
	}

	node->setTransform((TRS) { .position = position, });
	setNode("model", node, amuletModel);
}

void amuletPickup::update(entityManager *manager, float delta) {};
