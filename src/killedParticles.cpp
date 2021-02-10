#include <grend/geometryGeneration.hpp>
#include <stdlib.h>

#include "killedParticles.hpp"
#include "boxSpawner.hpp"
#include "player.hpp"
#include "timedLifetime.hpp"

// destructor for rtti
killedParticles::~killedParticles() {};

void killedParticles::onEvent(entityManager *manager, entity *ent, float delta) {
	//auto box = new player(manager, manager->engine, ent->node->transform.position + glm::vec3(0, 0, 1));
	auto box = new boxParticles(manager, ent->node->transform.position);

	manager->add(box);
}

boxParticles::boxParticles(entityManager *manager, glm::vec3 pos)
	: entity(manager)
{
	static gameObject::ptr model = nullptr;

	manager->registerComponent(this, "boxParticles", this);
	new timedLifetime(manager, this, 7.f);
	parts = std::make_shared<gameParticles>(32);

	for (unsigned i = 0; i < 32; i++) {
		velocities[i] = float(rand()) / RAND_MAX * 5.f;
		offsets[i]    = float(rand()) / RAND_MAX * 10.f;

		parts->positions[i] = (glm::mat4(1));
	}

	asdf = pos;
	parts->activeInstances = 32;
	parts->radius = 50.f;
	parts->update();

	if (!model) {
		gameModel::ptr mod = generate_cuboid(0.3, 0.3, 0.3);
		model = std::make_shared<gameObject>();
		compileModel("boxProjectile", mod);
		setNode("model", model, mod);
	}

	setNode("model", parts, model);
	setNode("parts", node, parts);
};

void boxParticles::update(entityManager *manager, float delta) {
	time += delta;

	for (unsigned i = 0; i < 32; i++) {
		glm::vec3 pos =
			glm::vec3(sin(offsets[i] * time),
		              velocities[i] * time,
		              cos(offsets[i]*time))
			+ asdf;

		parts->positions[i] = glm::translate(pos);
	}

	parts->update();
}