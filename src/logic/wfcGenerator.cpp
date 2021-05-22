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

static gameObject::ptr ladderModel = nullptr;
static gameObject::ptr coverModel = nullptr;

wfcGenerator::wfcGenerator(gameMain *game, std::string specFilename, unsigned seed) {
	if (!ladderModel || !coverModel) {
		ladderModel = loadSceneAsyncCompiled(game, DEMO_PREFIX "assets/obj/catacomb-tiles/ladder.glb");
		coverModel = loadSceneAsyncCompiled(game, DEMO_PREFIX "assets/obj/catacomb-tiles/ladder-cover.glb");

		auto redlit = std::make_shared<gameLightPoint>();
		auto greenlit = std::make_shared<gameLightPoint>();

		redlit->diffuse   = glm::vec4(1.0, 0.3, 0.01, 1.0);
		greenlit->diffuse = glm::vec4(0.3, 1.0, 0.01, 1.0);
		redlit->setTransform({ .position = glm::vec3(0, 2, 0) });
		greenlit->setTransform({ .position = glm::vec3(0, 2, 0) });

		setNode("lit", coverModel, redlit);
		setNode("lit", ladderModel, greenlit);
	}

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


	auto modelconf = genspec["models"];

	for (auto& [name, file] : modelconf.items()) {
		std::string objpath = dirnameStr(filename) + "/" + file.get<std::string>();
		models[name] = loadSceneAsyncCompiled(game, objpath);
	}

	std::map<size_t, size_t> fooidx;

	auto& tiles = genspec["tiles"];
	//TODO: don't want to do this if there might be multiple spec files loaded...
	stateClass.states.insert(0);

	for (unsigned i = 0; i < StateDef::Sockets; i++) {
		// empty space, empty space can connect with itself in every direction
		stateClass.socketmap[i][0].setState(0);
	}

	for (size_t idx = 0, vecidx = 1; idx < tiles.size(); idx++) {
		for (auto& em : tiles[idx]["rotations"]) {
			stateClass.states.insert(vecidx);

			std::string foo = tiles[idx]["name"].get<std::string>();
			int rot = em.get<int>();
			gameObject::ptr model = std::make_shared<gameObject>();
			model->setTransform({
				.rotation = glm::quat(glm::vec3(0, -M_PI/2 * rot, 0))
			});

			std::cerr << vecidx << "(" << idx << ")"
				<< " => " << tiles[idx]["name"]
				<< ", rotation " << foo << ":" << rot << std::endl;

			Name n = {foo, rot};
			nameToState[n] = vecidx;
			stateToName[vecidx] = n;
			fooidx[vecidx] = idx;

			setNode("model", model, models[tiles[idx]["model"]]);
			stateModels[vecidx] = model;

			if (tiles[idx].count("tags")) {
				for (auto& em : tiles[idx]["tags"]) {
					tags[em].insert(vecidx);
				}
			}

			vecidx++;
		}
	}

	for (auto& [idx, key] : stateToName) {
		for (auto& ent : tiles[fooidx[idx]]["adjacent"]) {
			int dir = ent[0].get<int>();
			std::string tilename = ent[1].get<std::string>();
			int rot = ent[2].get<int>();
			unsigned neighbor;

			auto& [ownname, ownrot] = key;

			if (rot < 0) {
				for (unsigned i = 0; i < StateDef::Sockets; i++) {
					int adjrot = (ownrot + i) % StateDef::Sockets;
					int adjdir = (dir + ownrot) % StateDef::Sockets;
					int oppdir = (adjdir + StateDef::Sockets/2) % StateDef::Sockets;

					Name neighborName = {tilename, adjrot};
					auto it = nameToState.find(neighborName);

					if (it == nameToState.end()) {
						std::cerr << "NOTE: state for ["
							<< neighborName.first
							<< ":" << neighborName.second
							<< "]" << "\n";
						continue;
					}

					neighbor = it->second;
					std::cerr << idx << " *> [" << adjdir << ", " << tilename
						<< " (" << neighbor << ":" << adjrot << ")" << "]"
						<< std::endl;

					stateClass.socketmap[adjdir][idx].setState(neighbor);
					stateClass.socketmap[oppdir][neighbor].setState(idx);
				}

			} else {
				int adjrot = (ownrot + rot) % StateDef::Sockets;
				int adjdir = (dir + ownrot) % StateDef::Sockets;
				int oppdir = (adjdir + StateDef::Sockets/2) % StateDef::Sockets;

				Name neighborName = {tilename, adjrot};
				auto it = nameToState.find(neighborName);

				if (it == nameToState.end()) {
					std::cerr << "Depends on invalid rotation state!\n";
					throw std::logic_error("Depends on invalid rotation state!");
				}

				neighbor = it->second;
				std::cerr << idx << " -> [" << adjdir << ":" << adjrot << ", " << tilename
					<< " (" << neighbor << ")" << "]" << std::endl;

				stateClass.socketmap[adjdir][idx].setState(neighbor);
				stateClass.socketmap[oppdir][neighbor].setState(idx);
			}
		}
	}

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

static void floodfill(uint8_t *data,
		int width, int height,
		int x, int y,
		uint8_t target, uint8_t replace)
{
	if (x >= width || x < 0 || y >= height || y < 0) {
		return;
	}

	uint8_t *cur = data + y*width + x;

	if (*cur == target) {
		*cur = replace;

		floodfill(data, width, height, x + 1, y, target, replace);
		floodfill(data, width, height, x - 1, y, target, replace);
		floodfill(data, width, height, x, y + 1, target, replace);
		floodfill(data, width, height, x, y - 1, target, replace);
	}
}

gameObject::ptr wfcGenerator::genCell(int x, int y, int z) {
	gameObject::ptr ret = std::make_shared<gameObject>();
	//WfcImpl *wfcgrid = getSector({x, y, z});

	bool valid = true;
	unsigned attempts = 0;
	std::random_device rd;
	std::mt19937 g(rd());

	srand(time(NULL));


retry:
	WFCSolver<StateDef, genwidth, genheight> wfcgrid(spec->stateClass);
	bsp::bspGen<genwidth, genheight> mapskel;
	uint8_t mapdata[genwidth * genheight];

	memset(mapdata, 0, sizeof(mapdata));
	mapskel.genSplits(7);
	mapskel.connectNodes(mapskel.root, mapdata, 20);

	auto leaves = mapskel.getLeafCenters();

	std::shuffle(leaves.begin(), leaves.end(), g);

	auto& entry = leaves[0];

	//mapdata[entry.second*genwidth + entry.first] = 0xe0;
	floodfill(mapdata, genwidth, genheight, entry.first, entry.second, 0xff, 0xe0);

	auto exitpoint = entry;
	bool connects = false;

	for (unsigned i = 1; i < leaves.size(); i++) {
		auto& leaf = leaves[i];
		auto dist = [] (auto& a, auto& b) {
			return abs(a.first - b.first) + abs(a.second - b.second);
		};

		if (mapdata[leaf.second*genwidth + leaf.first] == 0xe0) {
			int curdist = dist(exitpoint, entry);
			int leafdist = dist(leaf, entry);

			// look for maximally distant point
			if (leafdist > curdist) {
				exitpoint = leaf;
				connects = true;
				std::cerr << "got here" << std::endl;
			}
		}
	}

	if (!connects) {
		std::cerr << "BSP doesn't connect from entry point" << std::endl;
		goto retry;

	} else {
		std::cerr << "Connected, continuing to WFC generation..." << std::endl;
	}

	for (size_t bx = 0; bx < genwidth; bx++) {
		for (size_t by = 0; by < genheight; by++) {
			//if (mapdata[by*genwidth + bx] == 0xff) {
			if (mapdata[by*genwidth + bx]) {
				wfcgrid.setTile(bx, by, spec->tags["traversable"]);
			}
		}
	}

	std::cerr << "Set traversable paths" << std::endl;

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

	std::cerr << "Set empty parameter" << std::endl;

	for (bool running = true; running && valid;) {
		auto [r, v] = wfcgrid.iterate();
		running = r, valid = v;

		if (!valid) {
			break;
		}
	}

	std::cerr << "Generated WFC (valid: " << valid << ")" << std::endl;

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

						setNode("model", nobj, spec->stateModels[em]);
						setNode(objname, ret, nobj);
						break;
					}
				}
			}
		}
	}

	gameObject::ptr entryObj = std::make_shared<gameObject>();
	gameObject::ptr exitObj  = std::make_shared<gameObject>();

	entryObj->setTransform({.position = {4*entry.first, 0, 4*entry.second} });
	exitObj->setTransform({.position = {4*exitpoint.first, 0, 4*exitpoint.second} });

	setNode("model", entryObj, ladderModel);
	setNode("model", exitObj, coverModel);

	setNode("entry", ret, entryObj);
	setNode("exit", ret, exitObj);

	gameObject::ptr bspLeaves = std::make_shared<gameObject>();
	bspLeaves->visible = false;

	for (size_t i = 1; i < leaves.size(); i++) {
		std::string name = "node" + std::to_string(i);
		gameObject::ptr leaf = std::make_shared<gameObject>();
		leaf->setTransform({
			.position = {4*leaves[i].first, 0, 4*leaves[i].second}
		});
		setNode(name, bspLeaves, leaf);
	}

	setNode("leaves", ret, bspLeaves);

	return ret;
}

void wfcGenerator::generate(gameMain *game,
                            std::vector<glm::vec3> entries)
{
	auto cell = genCell(0, 0, 0);
	//cell->setTransform({ .position = {-genwidth * 2, 0, -genheight * 2}, });
	setNode("nodes", root, cell);

	// TODO: need to find a way to generate physics here
	mapobjs.clear();
	havePhysics = false;
}

void wfcGenerator::setPosition(gameMain *game, glm::vec3 position) {
	static bool food = false;

	if (!havePhysics) {
		game->phys->addStaticModels(nullptr, root, root->getTransformTRS(), mapobjs);
		havePhysics = true;
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
