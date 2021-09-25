#include <components/itemPickup.hpp>

template <class ParticleStruct, size_t maxParticles>
class particleSystem : public component, public updatable {
	static const size_t num = maxParticles;

	public:
		particleSystem(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			particleBuf = std::make_shared<gameParticles>(num);

			manager->registerComponent(ent, "particleSystem", this);
			manager->registerComponent(ent, "updatable", this);
		};

		virtual ~particleSystem() {};
		virtual void update(entityManager *manager, float delta) {
			for (size_t i = 0; i < maxParticles; i++) {
				this->integrate(particles[i], delta);
			}

			for (size_t i = 0; i < maxParticles; i++) {
				particleBuf->positions[i] = particles[i].transform;
			}

			particleBuf->activeInstances = num;
			particleBuf->radius = 100.f; // XXX
			particleBuf->update();
		}

		// TODO: system has a fixed number of particles,
		//       need a convenient way to adjust the number of active
		//       particles in the system
		virtual void spawn(ParticleStruct& particle) = 0;
		virtual void integrate(ParticleStruct& particle, float delta) = 0;

		gameParticles::ptr particleBuf;
		ParticleStruct particles[maxParticles];
};

struct particleInfo {
	glm::mat4 transform;
	glm::vec3 pos;
	float velocity;
	float offset;
	float age;
};

class explosionParticles : public particleSystem<particleInfo, 32> {
	public:
		explosionParticles(entityManager *manager, entity *ent);
		virtual ~explosionParticles();

		virtual void spawn(particleInfo& particle);
		virtual void integrate(particleInfo& particle, float delta);
};
