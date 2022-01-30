#include <components/explosionParticles.hpp>
#include <grend/gameEditor.hpp>

explosionParticles::~explosionParticles() {};

explosionParticles::explosionParticles(entityManager *manager, entity *ent)
	: particleSystem(manager, ent)
{
	static gameObject::ptr model = nullptr;
	manager->registerComponent(ent, this);

	for (auto& p : particles) {
		spawn(p);
	}

	if (!model) {
		//model = loadSceneAsyncCompiled(manager->engine, "assets/obj/emissive-cube.glb");
		//auto [data, _] = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/emissive-plane.glb");
		auto [data, _] = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/particle-fragment.glb");
		model = data;

		TRS t = model->getTransformTRS();
		t.scale = glm::vec3(0.2);
		model->setTransform(t);
	}

	setNode("model", particleBuf, model);
	// TODO: good idea? maybe not so much
	setNode("parts", ent->node, particleBuf);
}

void explosionParticles::spawn(particleInfo& particle) {
	auto randf = [] () { return float(rand()) / RAND_MAX; };

	particle.transform = glm::mat4(1);
	particle.pos       = glm::vec3(randf() - 0.5f, 0.f, randf() - 0.5f);
	particle.velocity  = randf() * 3.f;
	//particle.offset    = randf() * 5.f;
	particle.offset    = 0.f;
	particle.age       = 0.f;

	//offsets[idx]    = 0.f;
	// add an offset of up to 1 second so particles don't
	// all respawn at once
	//particle.age       = float(rand()) / RAND_MAX * 1.f;
}

void explosionParticles::integrate(particleInfo& particle, float delta) {
	particle.age += delta;
	float curVelocity = -0.9f * (particle.age*particle.age) / 2.f + particle.velocity;

	if (curVelocity <= -0.9f) {
		//particle.transform = glm::scale(glm::vec3(0.1));
		spawn(particle);
		return;
	}

	particle.offset += curVelocity * delta;
	glm::vec3 pos =
		//glm::vec3(0, velocities[i] * ages[i], 0);
		particle.pos + glm::vec3(0, particle.offset, 0)
		//+ asdf;
		;

	float scale = sin(particle.age + particle.offset);
	particle.transform =
		glm::translate(pos)
		* glm::mat4_cast(glm::quat(glm::vec3(0, particle.age, 0)))
		* glm::scale(glm::vec3(min(1.f + curVelocity, 1.f)))
		//* glm::mat4_cast(glm::quat(glm::vec3(ages[i], 0, 0)))
		/* glm::scale(glm::vec3(sin(particle.age + particle.offset) * 0.5f + 0.75f))*/
		;
	;
}
