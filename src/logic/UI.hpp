#pragma once

#include <components/health.hpp>
#include <logic/levelController.hpp>

#include <grend/ecs/ecs.hpp>
#include <grend/camera.hpp>

#include <nuklear/nuklear.h>

using namespace grendx;

void drawPlayerHealthbar(entityManager *manager,
                         struct nk_context *nk_ctx,
                         health *playerHealth);
void renderHealthbars(entityManager *manager,
                      struct nk_context *nk_ctx,
                      camera::ptr cam);
void renderObjectives(entityManager *manager,
                      levelController *level,
                      struct nk_context *nk_ctx);
void renderControls(gameMain *game, struct nk_context *nk_ctx);

class uiComponent : public component {
	public:
		uiComponent(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, this);
		}

		virtual ~uiComponent();

		// for updating any logic associated with the UI thing
		virtual void update() {};
		virtual bool active(entityManager *manager, entity *ent) { return true; }

		// for drawing the thing
		virtual void draw(entityManager *manager,
		                  entity *ent,
		                  struct nk_context *nk_ctx,
		                  camera::ptr cam) = 0;
};

class uiSystem : public entitySystem {
	public:
		typedef std::shared_ptr<uiSystem> ptr;
		typedef std::weak_ptr<uiSystem>   weakptr;

		uiSystem(struct nk_context *nk, camera::ptr c)
			: nk_ctx(nk), cam(c) {};

		virtual ~uiSystem();
		virtual void update(entityManager *manager, float delta);

		nk_context *nk_ctx;
		camera::ptr cam;
};

class dialogPrompt : public uiComponent {
	public:
		dialogPrompt(entityManager *manager,
		             entity *ent,
		             std::string _prompt)
			: uiComponent(manager, ent),
			  prompt(_prompt)
		{
			manager->registerComponent(ent, this);
		}

		virtual ~dialogPrompt();
		virtual bool active(entityManager *manager, entity *ent);
		//virtual void update();
		virtual void onEvent(entityManager *manager, entity *ent, entity *other);
		virtual void draw(entityManager *manager,
		                  entity *ent,
		                  struct nk_context *nk_ctx,
		                  camera::ptr cam);

	private:
		bool isActive = true;
		std::string prompt = "";
};
