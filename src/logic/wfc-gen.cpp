#include <grend/gameEditor.hpp>
#include <grend/geometryGeneration.hpp>
#include "wfcGenerator.hpp"
#include <wfc-test/wfc.hpp>
#include <math.h>

wfcGenerator::wfcGenerator(unsigned seed) {
	// TODO: do something with the seed
}

wfcGenerator::~wfcGenerator() {};

static constexpr float cellsize = 16;
static constexpr float gridsize = 5;

class statedef {
	public:
		using State = unsigned;
		static constexpr size_t maxStates = 256;

		// different implementations of StateSet here for benchmarking
		// TODO: template or something
#if 1
		class StateSet : public static_flagset<256> {
			public:
				size_t countStates() const { return size(); }
				bool hasState(const State& s) const { return count(s); }

				bool   anySet() const { return size() > 0; };
				void   setState(const State& s) { insert(s); };
				void   clearStates() { clear(); };
				void   unsetState(const State& s) { erase(s); };
				State  chooseState() {
					// TODO: for choosing random state
					size_t k = rand() % size();
					//auto em = *std::next(begin(), k);
					auto em = *(begin() + k);

					return em;
				}
		};
#endif

		static constexpr unsigned sockets = 4;
		std::unordered_map<State, StateSet> socketmap[sockets];
		//StateSet socketmap[sockets];
		StateSet states;

		void initializeTile(StateSet& s, uint64_t socketMask) {
			//for (State em = 0; em < maxStates; em++) {
			for (auto& em : states) {
				bool satisfied = true;

				for (unsigned bit = 0; bit < 4; bit++) {
					auto& smap = socketmap[bit];
					bool hasDir =
						smap.find(em) != smap.end() && smap[em].anySet();

					satisfied &= ((socketMask & (1 << bit)) && hasDir) || !((socketMask & (1 << bit)));
				}

				if (satisfied) {
					s.setState(em);
				}
			}
		}

		const StateSet& connects(const State& s, unsigned socket) const {
			static StateSet empty = {};
			auto it = socketmap[socket].find(s);

			if (it == socketmap[socket].end()) {
				//std::cerr << "NO BAD NO" << std::endl;
				return empty;

			} else {
				return it->second;
			}

			//return socketmap
		}
};


void wfcGenerator::generate(gameMain *game,
                            glm::vec3 curpos,
                            glm::vec3 lastpos)
{
	//static gameModel::ptr models[gridsize][gridsize];
	//static gameModel::ptr temp[gridsize][gridsize];
	// BIG XXX: avoid accessing landscape material shared pointer from multiple
	//          threads (assignment to mesh material increases use count)
	static std::mutex landscapemtx;

	gameObject::ptr ret = std::make_shared<gameObject>();
	std::list<std::future<bool>> futures;

	glm::vec3 diff = curpos - lastpos;
	float off = cellsize * (gridsize / 2);
	SDL_Log("curpos != genpos, diff: (%g, %g, %g)", diff.x, diff.y, diff.z);

#if 0
	for (int x = 0; x < gridsize; x++) {
		for (int y = 0; y < gridsize; y++) {
			int ax = x + diff.x;
			int ay = y + diff.z;

			if (models[x][y] == nullptr
			    || ax >= gridsize || ax < 0
			    || ay >= gridsize || ay < 0)
			{
				glm::vec3 coord =
					(curpos * cellsize)
					- glm::vec3(off, 0, off)
					+ glm::vec3(x*cellsize, 0, y*cellsize);

				glm::vec3 prev =
					(lastpos * cellsize)
					- glm::vec3(off, 0, off)
					+ glm::vec3(x*cellsize, 0, y*cellsize);

				if (models[x][y]) {
					// don't emit delete if there's no model there
					// (ie. on startup)
					emit((generatorEvent) {
							.type = generatorEvent::types::deleted,
							.position = prev + glm::vec3(cellsize*0.5, 0, cellsize*0.5),
							.extent = glm::vec3(cellsize * 0.5f, HUGE_VALF, cellsize*0.5f),
					});
				}

				emit((generatorEvent) {
					.type = generatorEvent::types::generatorStarted,
					.position = coord + glm::vec3(cellsize*0.5, 0, cellsize*0.5),
					.extent = glm::vec3(cellsize * 0.5f, HUGE_VALF, cellsize*0.5f),
				});

				SDL_Log("CCCCCCCC: generating coord (%g, %g)", coord.x, coord.z);

				// TODO: reaaaaallly need to split this up
				futures.push_back(game->jobs->addAsync([=] {
					SDL_Log("DDDDDDD: got here, from the future (%g, %g)",
							coord.x, coord.z);
					auto ptr = generateHeightmap(cellsize, cellsize, 2.0, coord.x, coord.z, landscapeThing);
					//auto ptr = generateHeightmap(24, 24, 0.5, coord.x, coord.z, thing);
					SDL_Log("EEEEEEE: generated model");
					ptr->transform.position = glm::vec3(coord.x, 0, coord.z);

					gameMesh::ptr mesh =
						std::dynamic_pointer_cast<gameMesh>(ptr->getNode("mesh"));
					gameMesh *fug = dynamic_cast<gameMesh*>(ptr->getNode("mesh").get());

					gameMesh *blarg = (gameMesh*)ptr->getNode("mesh").get();

					SDL_Log("EEEEEEE.v2: has node: %d, mesh ptr: %p, source ptr: %p, other? %p, id: %s, blarg: %p",
						ptr->hasNode("mesh"), mesh.get(),
						ptr->getNode("mesh").get(), fug, ptr->getNode("mesh")->idString().c_str(), blarg);

					if (mesh) {
						//std::lock_guard<std::mutex> g(landscapemtx);
						mesh->meshMaterial = std::make_shared<material>();
						mesh->meshMaterial->factors.diffuse = {0.15, 0.3, 0.1, 1};
						mesh->meshMaterial->factors.ambient = {1, 1, 1, 1};
						mesh->meshMaterial->factors.specular = {1, 1, 1, 1};
						mesh->meshMaterial->factors.emissive = {0, 0, 0, 0};
						mesh->meshMaterial->factors.roughness = 0.9f;
						mesh->meshMaterial->factors.metalness = 0.f;
						mesh->meshMaterial->factors.opacity = 1;
						mesh->meshMaterial->factors.refract_idx = 1.5;
					}
					//mesh->meshMaterial = landscapeMaterial;
					std::string name = "gen["+std::to_string(int(x))+"]["+std::to_string(int(y))+"]";
					SDL_Log("FFFFFFF: setting node");

					gameObject::ptr foo = std::make_shared<gameObject>();
					setNode("asdfasdf", foo, ptr);
					SDL_Log("GGGGGGG: Adding landscape mesh to physics");

					auto fut = game->jobs->addDeferred([=]{
						SDL_Log("HHHHHHH: Generating new landscape model");
						compileModel(name, ptr);
						bindModel(ptr);
						return true;
					});

					SDL_Log("IIIIIII: generating tree instances");
					glm::vec2 posgrad = randomGradient(glm::vec2(coord.x, coord.z));
					float baseElevation = landscapeThing(coord.x, coord.z);
					int randtrees = (posgrad.x + 1.0)*0.5 * 5 * (1.0 - baseElevation/50.0);

					game->phys->addStaticModels(nullptr, foo, TRS());

					gameParticles::ptr parts = std::make_shared<gameParticles>(32);
					parts->activeInstances = randtrees;
					parts->radius = cellsize / 2.f * 1.415;

					for (unsigned i = 0; i < parts->activeInstances; i++) {
						TRS transform;
						glm::vec2 pos = randomGradient(glm::vec2(coord.x + i, coord.z + i));

						float tx = ((pos.x + 1)*0.5) * cellsize;
						float ty = ((pos.y + 1)*0.5) * cellsize;

						transform.position = glm::vec3(
							tx, landscapeThing(coord.x + tx, coord.z + ty) - 0.1, ty
						);
						transform.scale = glm::vec3((posgrad.y + 1.0)*0.5*3.0+0.5);
						parts->positions[i] = transform.getTransform();
					}

					SDL_Log("JJJJJJJ: adding node instances...");
					parts->update();
					setNodeXXX("tree", parts, treeNode);
					setNodeXXX("parts", ptr, parts);
					SDL_Log("KKKKKKK: ok cool");

					temp[x][y] = ptr;
					fut.wait();
					return true;
				}));

				emit((generatorEvent) {
					.type = generatorEvent::types::generated,
					.position = coord + glm::vec3(cellsize*0.5, 0, cellsize*0.5),
					.extent = glm::vec3(cellsize*0.5f, HUGE_VALF, cellsize*0.5f),
				});

			} else {
				temp[x][y] = models[ax][ay];
			}
		}
	}
#endif

	for (auto& fut : futures) {
		fut.wait();
	}

#if 0
	auto meh = game->jobs->addDeferred([&] {
		for (int x = 0; x < gridsize; x++) {
			for (int y = 0; y < gridsize; y++) {
				models[x][y] = temp[x][y];
				temp[x][y] = nullptr;
				std::string name = "gen["+std::to_string(int(x))+"]["+std::to_string(int(y))+"]";
				setNode(name, ret, models[x][y]);
			}
		}

		return true;
	});
	meh.wait();

	/*
	auto fut = game->jobs->addDeferred([&] {
		//bindCookedMeshes();
		return true;
	});

	fut.wait();
	*/
#endif
	returnValue = ret;
}

void wfcGenerator::setPosition(gameMain *game, glm::vec3 position) {
	glm::vec3 curpos = glm::floor((glm::vec3(1, 0, 1)*position)/cellsize);

	if (genjob.valid() && genjob.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
		genjob.get();
		setNode("nodes", root, returnValue);
		returnValue = nullptr;
	}

	if (!genjob.valid() && curpos != lastPosition) {
		SDL_Log("BBBBBBBBBB: starting new generator job");
		glm::vec3 npos = lastPosition;
		lastPosition = curpos;

		genjob = game->jobs->addAsync([=] {
			generate(game, curpos, npos);
			return true;
		});
	}
}
