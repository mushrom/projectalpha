#include <grend/gameEditor.hpp>
#include <grend/geometryGeneration.hpp>
#include <grend/utility.hpp>

#include "wfcGenerator.hpp"
#include <math.h>
#include <nlohmann/json.hpp>
#include <iostream>

using namespace wfc;

static constexpr int genwidth = wfcGenerator::genwidth;
static constexpr int genheight = wfcGenerator::genheight;
static constexpr float cellsize = (wfcGenerator::genwidth - 1)*4;
static constexpr int gridsize = 3;
static constexpr int foo = (genwidth - 1)*4;

wfcGenerator::wfcGenerator(gameMain *game, std::string specFilename, unsigned seed) {
	//parseJson(DEMO_PREFIX "assets/obj/ld48/tiles/wfc-config.json");
	spec.reset(new wfcSpec(game, specFilename));
	generate(game, {});
	//static std::vector<physicsObject::ptr> mapobjs;
	//static std::unique_ptr<std::vector<physicsObject::ptr>>
	//
	//gameObject::ptr cell = genCell(0, 0, 0);
	//cell->setTransform({ .position = {-genwidth * 2, 0, -genheight * 2}, });

	//setNode("nodes", root, cell);
	//TRS foo = cell->getTransformTRS();
	//game->phys->addStaticModels(nullptr, cell, foo, mapobjs);
	//mapobjs.clear();

	/*
	// XXX: initialize a parameter of empty sectors around the map, to limit
	//      the map size (should result in walls at boundary)
	for (int i = -gridsize; i < gridsize; i++) {
		for (auto c : {wfcGenerator::Coord { i*foo,        0,  gridsize*foo},
		               wfcGenerator::Coord { gridsize*foo, 0,  i*foo},
		               wfcGenerator::Coord { i*foo,        0, -gridsize*foo},
		               wfcGenerator::Coord {-gridsize*foo, 0,  i*foo}})
		{
			sectors[c].reset(new WfcImpl(stateClass));
			auto& wfcgrid = sectors[c];
			for (unsigned x = 0; x < genwidth; x++) {
				for (unsigned z = 0; z < genheight; z++) {
					wfcgrid->gridState.tiles[z*genheight + x].clearStates();
					wfcgrid->gridState.tiles[z*genheight + x].setState(0);
				}
			}
		}
	}
	*/
}

wfcSpec::wfcSpec(gameMain *game, std::string filename) {
	parseJson(game, filename);
}

void wfcSpec::parseJson(gameMain *game, std::string filename) {
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

	//std::map<std::string, size_t> keyidx;
	//std::map<size_t, std::string> idxkey;
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

		nameToState[tiles[idx]["name"]] = vecidx;
		stateToName[vecidx] = tiles[idx]["name"];
		fooidx[vecidx] = idx;

		std::string objpath = dirnameStr(filename) + "/" + objfname;
		//gameObject::ptr objModel = loadSceneAsyncCompiled(game, objpath);
		//models.push_back(objModel);
		models.push_back(loadSceneAsyncCompiled(game, objpath));
	}

	for (auto& [idx, key] : stateToName) {
		for (auto& ent : tiles[fooidx[idx]]["adjacent"]) {
			int dir = ent[0];
			std::string name = ent[1];
			unsigned neighbor = nameToState[name];

			std::cerr << idx << " -> [" << dir << ", " << name
				<< " (" << neighbor << ")" << "]" << std::endl;

			stateClass.socketmap[dir][idx].setState(nameToState[name]);
			stateClass.socketmap[(dir + 2)%4][nameToState[name]].setState(idx);
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

#include <logic/bspGenerator.hpp>

gameObject::ptr wfcGenerator::genCell(int x, int y, int z) {
	gameObject::ptr ret = std::make_shared<gameObject>();
	//WfcImpl *wfcgrid = getSector({x, y, z});
	WFCSolver<StateDef, genwidth, genheight> wfcgrid(spec->stateClass);

	bool valid = true;
	unsigned attempts = 0;

	srand(time(NULL));


retry:
	bsp::bspGen<genwidth, genheight> mapskel;
	uint8_t mapdata[genwidth * genheight];

	memset(mapdata, 0, sizeof(mapdata));
	mapskel.genSplits(7);
	mapskel.connectNodes(mapskel.root, mapdata, 24);

	for (size_t bx = 0; bx < genwidth; bx++) {
		for (size_t by = 0; by < genheight; by++) {
			if (mapdata[by*genwidth + bx]) {
				//wfcgrid.gridState.tiles[by*genwidth + bx].clearStates();
				//wfcgrid.gridState.tiles[by*genwidth + bx].setState(spec->nameToState["floor-tile-empty"]);
				wfcgrid.setTile(bx, by, spec->nameToState["floor-tile-empty"]);
			}
		}
	}

	for (unsigned idx = 0; idx < genwidth && idx < genheight; idx++) {
		for (auto c : { idx,
		                idx*genwidth,
		                idx*genwidth + (genwidth - 1),
		                genheight*(genwidth - 1) + idx })
		{
			wfcgrid.gridState.tiles[c].clearStates();
			wfcgrid.gridState.tiles[c].setState(0);
		}
	}

	for (bool running = true; running && valid;) {
		auto [r, v] = wfcgrid.iterate();
		running = r, valid = v;

		if (!valid) {
			break;
		}
	}

	if (!valid && attempts < 50) {
		wfcgrid.reset();
		attempts++;
		valid = true;
		goto retry;

	} else {
		static std::mutex mtx;
		std::lock_guard g(mtx);

		for (size_t gx = 0; gx < genwidth - 1; gx++) {
			for (size_t gy = 0; gy < genheight - 1; gy++) {
				auto& tile = wfcgrid.gridState.tiles[gy*genwidth + gx];

				for (auto& em : tile) {
					// BIG XXX: avoid accessing shared pointer from multiple
					//          threads (assignment increases use count)
					if (em) {
						std::string objname = std::string("tile")
							+ "[" + std::to_string(gx) + "]" 
							+ "[" + std::to_string(gy) + "]";

						gameObject::ptr nobj = std::make_shared<gameObject>();
						TRS transform = nobj->getTransformTRS();
						transform.position = glm::vec3(4*gx, 0, 4*gy);
						nobj->setTransform(transform);

						setNode("model", nobj, spec->models[em]);
						setNode(objname, ret, nobj);
						break;
					}
				}
			}
		}
	}

	return ret;
}

void wfcGenerator::generate(gameMain *game,
                            std::vector<glm::vec3> entries)
{
	auto cell = genCell(0, 0, 0);
	cell->setTransform({ .position = {-genwidth * 2, 0, -genheight * 2}, });
	setNode("nodes", root, cell);
}

void wfcGenerator::setPosition(gameMain *game, glm::vec3 position) {
	static bool food = false;

	if (!food) {
		static std::vector<physicsObject::ptr> *mapobjs;
		mapobjs = new std::vector<physicsObject::ptr>();
		game->phys->addStaticModels(nullptr, root, root->getTransformTRS(), *mapobjs);
		food = true;
	}

#if 0
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
#endif
}
