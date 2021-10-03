#include <components/itemPickup.hpp>
#include <components/inventory.hpp>
#include <utility/serializer.hpp>
#include <logic/gameController.hpp>
#include <grend/gameEditor.hpp>

#include <logic/UI.hpp>

pickupAction::~pickupAction() {};
autopickupAction::~autopickupAction() {};
hasItem::~hasItem() {};
pickup::~pickup() {};
autopickup::~autopickup() {};

void pickupAction::onEvent(entityManager *manager, entity *ent, entity *other) {
	inventory *inv;
	castEntityComponent(inv, manager, ent, "inventory");

	if (!inv) {
		return;
	}

	const uint8_t *keystates = SDL_GetKeyboardState(NULL);

	// big-ish XXX: ideally input events should all be dispatched through handlers,
	//              but that would be much messier... being able to poll for state
	//              here is conceptually much easier to work with
	if (keystates[SDL_SCANCODE_X] || SDL_GameControllerGetButton(Controller(), SDL_CONTROLLER_BUTTON_X))
	{
		// XXX: manager->engine
		SDL_Log("Picking up an item!");
		inv->insert(manager, other);
		//manager->remove(other);
		new hasItem(manager, ent, tags);

		Messages()->publish({
			.type = "itemPickedUp",
			.ent  = other,
			.comp = ent,
			// TODO: tag with level
		});
	}
}

void autopickupAction::onEvent(entityManager *manager, entity *ent, entity *other) {
	inventory *inv;
	castEntityComponent(inv, manager, ent, "inventory");

	if (!inv) {
		return;
	}

	const uint8_t *keystates = SDL_GetKeyboardState(NULL);

	// big-ish XXX: ideally input events should all be dispatched through handlers,
	//              but that would be much messier... being able to poll for state
	//              here is conceptually much easier to work with
	// XXX: manager->engine

	SDL_Log("Picking up an item!");

	
	autopickup *ap;
	castEntityComponent(ap, manager, other, "autopickup");

	if (!ap) {
		return;
	}

	ap->onEvent(manager, ap, ent);
	manager->remove(other);

	//inv->insert(manager, other);
	//manager->remove(other);
	//new hasItem(manager, ent, tags);

	Messages()->publish({
		.type = "itemPickedUp",
		.ent  = other,
		.comp = ent,
		// TODO: tag with level
	});
}


struct particleInfo {
	glm::mat4 transform;
	float velocity;
	float offset;
	float age;
};

// TODO: generic particle system
class fragmentParticles : public component, public updatable {
	static const unsigned num = 32;

	public:
		fragmentParticles(entityManager *manager, entity *ent);
		virtual ~fragmentParticles();
		virtual void update(entityManager *manager, float delta);

		void spawn(particleInfo& particle) {
			particle.transform = glm::mat4(1);
			particle.velocity  = float(rand()) / RAND_MAX * 2.5f;
			particle.offset    = float(rand()) / RAND_MAX * 5.f;
			//offsets[idx]    = 0.f;
			// add an offset of up to 1 second so particles don't
			// all respawn at once
			particle.age       = float(rand()) / RAND_MAX * 1.f;
		}

		void updateParticle(particleInfo& particle, float delta);

		gameParticles::ptr particleBuf;
		particleInfo particles[num];
		//float velocities[num];
		//float offsets[num];
		//float ages[num];
		float time = 0.0;

		glm::vec3 asdf;
};

fragmentParticles::~fragmentParticles() {};

fragmentParticles::fragmentParticles(entityManager *manager, entity *ent)
	: component(manager, ent)
{
	static gameObject::ptr model = nullptr;

	manager->registerComponent(ent, "fragmentParticles", this);
	manager->registerComponent(ent, "updatable", this);

	//new timedLifetime(manager, this, 7.f);
	particleBuf = std::make_shared<gameParticles>(num);
	glm::vec3 pos = ent->node->getTransformTRS().position;

	for (unsigned i = 0; i < num; i++) {
		spawn(particles[i]);
		particleBuf->positions[i] = particles[i].transform;
		//parts->positions[i] = (glm::mat4(1));
		//parts->positions[i] = (glm::vec4(1));
	}

	//asdf = pos;
	asdf = glm::vec3(0);
	particleBuf->activeInstances = num;
	particleBuf->radius = 100.f;
	particleBuf->update();

	if (!model) {
		//model = loadSceneAsyncCompiled(manager->engine, "assets/obj/emissive-cube.glb");
		//auto [data, _] = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/emissive-plane.glb");
		auto [data, _] = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/particle-fragment.glb");
		model = data;
	}

	setNode("model", particleBuf, model);
	// TODO: good idea? maybe not so much
	setNode("parts", ent->node, particleBuf);
};

void fragmentParticles::update(entityManager *manager, float delta) {
	time += delta;

	for (unsigned i = 0; i < num; i++) {
		updateParticle(particles[i], delta);
		particleBuf->positions[i] = particles[i].transform;
	}

	particleBuf->update();
}

void fragmentParticles::updateParticle(particleInfo& particle, float delta) {
	if (particle.age > 5.0) {
		// destroy and respawn
		spawn(particle);
	}

	glm::vec3 pos =
		//glm::vec3(0, velocities[i] * ages[i], 0);
		glm::vec3(sin(particle.offset * particle.age),
				particle.velocity * particle.age,
				cos(particle.offset * particle.age))
		//+ asdf;
		;

	particle.age += delta;

	float scale = sin(particle.age + particle.offset);
	particle.transform =
		glm::translate(pos)
		* glm::mat4_cast(glm::quat(glm::vec3(0, particle.age, 0)))
		//* glm::mat4_cast(glm::quat(glm::vec3(ages[i], 0, 0)))
		* glm::scale(glm::vec3(sin(particle.age + particle.offset) * 0.5f + 0.75f));
	;
}

pickup::pickup(entityManager *manager, glm::vec3 position)
	: pickup(manager, this, setSerializedPosition<pickup>(position)) {};

pickup::pickup(entityManager *manager, entity *ent, nlohmann::json properties)
	: entity(manager, properties)
{
	new areaSphere(manager, this, 2.f);
	new dialogPrompt(manager, this, "[X] Pick up the item here");
	new fragmentParticles(manager, this);

	manager->registerComponent(this, "pickup", this);
}

autopickup::autopickup(entityManager *manager, glm::vec3 position)
	: autopickup(manager, this,
	             setSerializedPosition<autopickup>(position)) {};

autopickup::autopickup(entityManager *manager, entity *ent, nlohmann::json properties)
	: entity(manager, properties)
{
	new areaSphere(manager, this, 2.f);
	//new fragmentParticles(manager, this);

	manager->registerComponent(this, "autopickup", this);
}

void autopickup::onEvent(entityManager *manager, entity *ent, entity *other) {
}
