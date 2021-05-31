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
