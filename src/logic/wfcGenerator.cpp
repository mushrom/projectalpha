#include <grend/gameEditor.hpp>
#include <grend/geometryGeneration.hpp>
#include <grend/utility.hpp>

#include "wfcGenerator.hpp"
#include <math.h>
#include <nlohmann/json.hpp>
#include <iostream>

using namespace wfc;

wfcGenerator::wfcGenerator(gameMain *game, std::string spec, unsigned seed) {
	//parseJson(DEMO_PREFIX "assets/obj/ld48/tiles/wfc-config.json");
	parseJson(game, spec);
}

void wfcGenerator::parseJson(gameMain *game, std::string filename) {
	nlohmann::json genspec;

	std::ifstream foo(filename);
	SDL_Log("Got here!");

	if (foo.good()) {
		try {
			SDL_Log("opened spec!");
			foo >> genspec;
			std::cerr << genspec << std::endl;

		} catch (std::exception& e) {
			std::cerr << "Error parsing wfc spec: " << e.what() << std::endl;
			throw std::logic_error(e.what());
		}

	} else {
		SDL_Log("Couldn't open json wfc spec!");
		throw std::logic_error("Couldn't open json wfc spec!");
	}

	std::map<std::string, size_t> keyidx;
	std::map<size_t, std::string> idxkey;
	std::map<size_t, size_t> fooidx;

	auto& tiles = genspec["tiles"];
	//TODO: don't want to do this if there might be multiple spec files loaded...
	stateClass.states.insert(0);
	models.push_back(nullptr);

	for (unsigned i = 0; i < StateDef::Sockets; i++) {
		// empty space, empty space can connect with itself in every direction
		stateClass.socketmap[i][0].setState(0);
	}

	for (size_t idx = 0; idx < tiles.size(); idx++) {
		std::string objfname = tiles[idx]["file"];
		//size_t vecidx = models.size();
		size_t vecidx = idx + 1;

		stateClass.states.insert(vecidx);
		std::cerr << vecidx << "(" << idx << ")"
			<< " => " << tiles[idx]["name"]
			<< " @ " << objfname << std::endl;

		keyidx[tiles[idx]["name"]] = vecidx;
		idxkey[vecidx] = tiles[idx]["name"];
		fooidx[vecidx] = idx;

		std::string objpath = dirnameStr(filename) + "/" + objfname;
		//gameObject::ptr objModel = loadSceneAsyncCompiled(game, objpath);
		//models.push_back(objModel);
		models.push_back(loadSceneAsyncCompiled(game, objpath));
	}

	for (auto& [idx, key] : idxkey) {
		for (auto& ent : tiles[fooidx[idx]]["adjacent"]) {
			int dir = ent[0];
			std::string name = ent[1];
			unsigned neighbor = keyidx[name];

			std::cerr << idx << " -> [" << dir << ", " << name
				<< " (" << neighbor << ")" << "]" << std::endl;

			stateClass.socketmap[dir][idx].setState(keyidx[name]);
			stateClass.socketmap[(dir + 2)%4][keyidx[name]].setState(idx);
		}
	}

#if 1
	for (unsigned dir = 0; dir < StateDef::Sockets; dir++) {
		for (auto& em : stateClass.states) {
			auto& smap = stateClass.socketmap[dir];

			if (smap.find(em) == smap.end() || !smap[em].anySet()) {
				// if no rules for this direction, add a rule for the empty state
				smap[em].setState(0);
				stateClass.socketmap[(dir+2)%4][0].setState(em);
			}
		}
	}
#endif

	// TODO: do something with the seed
}

wfcGenerator::~wfcGenerator() {};

static constexpr int genwidth = 10;
static constexpr int genheight = 10;
static constexpr float cellsize = (genwidth - 1)*4;
static constexpr int gridsize = 3;

static inline void copyX(wfcGenerator::WfcImpl *a, wfcGenerator::WfcImpl *b, bool dir) {
	std::cerr << "copying X!" << std::endl;

	for (size_t x = 0; x < 10; x++) {
		size_t lower = x;
		size_t upper = (genheight - 1)*genwidth + x;

		size_t to_idx   = dir? upper : lower;
		size_t from_idx = dir? lower : upper;

		if (b->gridState.tiles[from_idx].countStates() > 1) {
			std::cerr << "Probably invalid! (copyX)" << std::endl;
		}

		if (b->gridState.tiles[from_idx].countStates() == 0) {
			std::cerr << "zero tile! (copyX)" << std::endl;
		}

		a->gridState.tiles[to_idx] = b->gridState.tiles[from_idx];
	}
}

static inline void copyZ(wfcGenerator::WfcImpl *a, wfcGenerator::WfcImpl *b, bool dir) {
	for (size_t z = 0; z < 10; z++) {
		size_t lower = z * genwidth;
		size_t upper = (z * genwidth) + (genwidth - 1);

		size_t to_idx   = dir? upper : lower;
		size_t from_idx = dir? lower : upper;

		a->gridState.tiles[to_idx] = b->gridState.tiles[from_idx];
	}
}

wfcGenerator::WfcImpl *wfcGenerator::getSector(const wfcGenerator::Coord& coord) {
	WfcImpl *ret = nullptr;
	auto it = sectors.find(coord);
	const auto& [cx, cy, cz] = coord;

	if (it == sectors.end() || it->second == nullptr) {
		std::cerr << "Generating new sector!" << std::endl;
		sectors[coord].reset(new WfcImpl(stateClass));
		//sectors[coord] = std::make_shared<WfcImpl>(stateClass);
		sectors[coord]->backtrackDepth = 5;

		for (int x = -1; x <= 1; x += 2) {
			auto other = sectors.find({cx + x*cellsize, cy, cz});

			if (other != sectors.end() && other->second != nullptr) {
				copyZ(sectors[coord].get(), other->second.get(), x > 0);
			}
		}

		for (int z = -1; z <= 1; z += 2) {
			auto other = sectors.find({cx, cy, cz + z*cellsize});

			if (other != sectors.end() && other->second != nullptr) {
				copyX(sectors[coord].get(), other->second.get(), z > 0);
			}
		}
	}

	ret = sectors[coord].get();

	return ret;
}

gameObject::ptr wfcGenerator::genCell(int x, int y, int z) {
	gameObject::ptr ret = std::make_shared<gameObject>();
	WfcImpl *wfcgrid = getSector({x, y, z});
	//WFCSolver<StateDef, genwidth, genheight> wfcgrid(stateClass);

	bool valid = true;
	for (bool running = true; running && valid;) {
		auto [r, v] = wfcgrid->iterate();
		running = r, valid = v;

		if (!valid) {
			break;
		}
	}

	if (valid) {
		for (size_t gx = 0; gx < genwidth - 1; gx++) {
			for (size_t gy = 0; gy < genheight - 1; gy++) {
				auto& tile = wfcgrid->gridState.tiles[gy*genwidth + gx];

				for (auto& em : tile) {
					// BIG XXX: avoid accessing shared pointer from multiple
					//          threads (assignment increases use count)
					static std::mutex mtx;
					std::lock_guard g(mtx);

					std::string objname = std::string("tile")
						+ "[" + std::to_string(gx) + "]" 
						+ "[" + std::to_string(gy) + "]";

					if (em) {
						gameObject::ptr nobj = std::make_shared<gameObject>();
						TRS transform = nobj->getTransformTRS();
						transform.position = glm::vec3(4*gx, 0, 4*gy);
						nobj->setTransform(transform);

						setNode("model", nobj, models[em]);
						setNode(objname, ret, nobj);
					}
				}
			}
		}

	} else {
		SDL_Log("Couldn't generate new tile!");
	}

	return ret;
}

void wfcGenerator::generate(gameMain *game,
                            glm::vec3 curpos,
                            glm::vec3 lastpos)
{
	static gameObject::ptr grid[gridsize][gridsize];
	static gameObject::ptr temp[gridsize][gridsize];

	using physvecPtr = std::unique_ptr<std::vector<physicsObject::ptr>>;

	// XXX:
	static physvecPtr tempobjs[gridsize][gridsize];
	static physvecPtr gridobjs[gridsize][gridsize];

	gameObject::ptr ret = std::make_shared<gameObject>();
	std::list<std::future<bool>> futures;

	glm::vec3 diff = curpos - lastpos;
	float off = cellsize * (gridsize / 2);
	SDL_Log("curpos != genpos, diff: (%g, %g, %g)", diff.x, diff.y, diff.z);

#if 1
	for (int x = 0; x < gridsize; x++) {
		for (int y = 0; y < gridsize; y++) {
			int ax = x + diff.x;
			int ay = y + diff.z;

			if (grid[x][y] == nullptr
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

				if (grid[x][y]) {
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
				//futures.push_back(game->jobs->addAsync([=, this] {
				{

					SDL_Log("DDDDDDD: got here, from the future (%g, %g)",
							coord.x, coord.z);
					//auto ptr = generateHeightmap(cellsize, cellsize, 2.0, coord.x, coord.z, landscapeThing);
					auto ptr = genCell(coord.x, 0, coord.z);

					//auto ptr = generateHeightmap(24, 24, 0.5, coord.x, coord.z, thing);
					SDL_Log("EEEEEEE: generated model");
					//ptr->getTransformTRS().position = glm::vec3(coord.x, 0, coord.z);
					TRS transform = ptr->getTransformTRS();
					transform.position = glm::vec3(coord.x, 0, coord.z);
					ptr->setTransform(transform);

					/*
					auto probe = std::make_shared<gameIrradianceProbe>();
					probe->setTransform((TRS) { .position = {0, 1, 0}, });
					setNode("probe", ptr, probe);
					*/

					/*
					gameMesh::ptr mesh =
						std::dynamic_pointer_cast<gameMesh>(ptr->getNode("mesh"));
					gameMesh *fug = dynamic_cast<gameMesh*>(ptr->getNode("mesh").get());

					gameMesh *blarg = (gameMesh*)ptr->getNode("mesh").get();

					SDL_Log("EEEEEEE.v2: has node: %d, mesh ptr: %p, source ptr: %p, other? %p, id: %s, blarg: %p",
						ptr->hasNode("mesh"), mesh.get(),
						ptr->getNode("mesh").get(), fug, ptr->getNode("mesh")->idString().c_str(), blarg);
						*/

					//auto meh = game->jobs->addDeferred([=, this] {
						//game->phys->addStaticModels(nullptr, ptr, TRS(), objs);
					tempobjs[x][y] = std::make_unique<std::vector<physicsObject::ptr>>();

					game->phys->addStaticModels(nullptr, ptr, transform, *(tempobjs[x][y].get()));
						//return true;
					////});

					temp[x][y] = ptr;
					//fut.wait();
					//return true;
				}
				//}));

				emit((generatorEvent) {
					.type = generatorEvent::types::generated,
					.position = coord + glm::vec3(cellsize*0.5, 0, cellsize*0.5),
					.extent = glm::vec3(cellsize*0.5f, HUGE_VALF, cellsize*0.5f),
				});

			} else {
				temp[x][y] = grid[ax][ay];
				tempobjs[x][y] = std::move(gridobjs[ax][ay]);
			}
		}
	}

	for (auto& fut : futures) {
		fut.wait();
	}
#endif

	std::future<bool> syncFutures[gridsize][gridsize];
	for (int x = 0; x < gridsize; x++) {
		for (int y = 0; y < gridsize; y++) {
			syncFutures[x][y] = game->jobs->addDeferred([&, x, y] {
				grid[x][y] = temp[x][y];
				gridobjs[x][y] = std::move(tempobjs[x][y]);

				temp[x][y] = nullptr;
				std::string name = "gen["+std::to_string(int(x))+"]["+std::to_string(int(y))+"]";
				setNode(name, ret, grid[x][y]);
				return true;
			});
		}
	}

	for (unsigned x = 0; x < gridsize; x++) {
		for (unsigned y = 0; y < gridsize; y++) {
			syncFutures[x][y].wait();
		}
	}

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

		/*
		genjob = game->jobs->addAsync([=] {
			generate(game, curpos, npos);
			return true;
		});
		*/
		genjob = game->jobs->addAsync([=, this] {
			generate(game, curpos, npos);
			return true;
		});
	}
}
