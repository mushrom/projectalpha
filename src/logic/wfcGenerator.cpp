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
		//ladderModel = loadSceneAsyncCompiled(game, DEMO_PREFIX "assets/obj/catacomb-tiles/ladder.glb");
		//coverModel = loadSceneAsyncCompiled(game, DEMO_PREFIX "assets/obj/catacomb-tiles/ladder-cover.glb");

		ladderModel = loadSceneAsyncCompiled(game, DEMO_PREFIX "assets/obj/catacomb-tiles/ascending-staircase.gltf");
		coverModel = loadSceneAsyncCompiled(game, DEMO_PREFIX "assets/obj/catacomb-tiles/descending-staircase.gltf");
		
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

/*
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

static void clearUnreachable(uint8_t *data,
                             int width, int height,
                             uint8_t target, uint8_t replace)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			uint8_t *cur = data + y*width + x;

			if (*cur != target) {
				*cur = replace;
			}
		}
	}
}
*/

using Coord = std::pair<int, int>;
template <int X, int Y>
static int markShortestPath(array2D<int, X, Y>& arr,
                            Coord start, Coord end,
                            uint8_t replace)
{
	std::set<Coord> unvisited;
	std::set<Coord> visited;
	int distances[X][Y];
	Coord previous[X][Y];

	int ret = INT_MAX;

	for (int x = 0; x < X; x++) {
		for (int y = 0; y < Y; y++) {
			distances[x][y] = INT_MAX;
			previous[x][y] = {-1, -1};
		}
	}

	unvisited.insert(start);
	distances[start.first][start.second] = 0;

	while (!unvisited.empty()) {
		Coord current = *unvisited.begin();
		unvisited.erase(current);

		if (distances[current.first][current.second] == INT_MAX) {
			// no path to end point
			break;
		}

		for (auto [x, y] : {Coord {-1, 0},
		                    Coord {0, -1},
		                    Coord {1, 0},
		                    Coord {0, 1}})
		{
			Coord coord = {current.first + x, current.second + y};

			if (arr.valid(coord.first, coord.second)
				&& arr.get(coord.first, coord.second)
				&& !visited.count(coord))
			{
				int  dist = distances[current.first][current.second] + 1;
				int& other = distances[coord.first][coord.second];

				if (dist < other) {
					other = dist;
					previous[coord.first][coord.second] = current;
				}

				unvisited.insert(coord);
			}
		}

		visited.insert(current);
		if (current == end) {
			// end point reached, no point in continuing
			ret = distances[current.first][current.second];
			break;
		}
	}

	if (ret < INT_MAX) {
		Coord c = end;

		while (c != start && c != Coord{-1, -1}) {
			arr.set(c.first, c.second, replace);
			//uint8_t *cur = data + c.second*width + c.first;

			//*cur = replace;
			c = previous[c.first][c.second];
		}
	}

	return ret;
}

template <int X, int Y>
static Coord generateDijkstraMap(array2D<float, X, Y>& distances,
                                 array2D<bool, X, Y>& mask,
                                 Coord start)
{
	std::set<Coord> unvisited;
	std::set<Coord> visited;

	unvisited.insert(start);
	distances.set(start, 0);

	float maxDist = 0.f;
	Coord maxCoord = start;

	while (!unvisited.empty()) {
		float min = HUGE_VALF;
		Coord current;

		// XXX: should store this in sorted order somehow...
		for (auto& c : unvisited) {
			if (distances.get(c) < min) {
				min = distances.get(c);
				current = c;
			}
		}
		unvisited.erase(current);

		if (distances.get(current) == HUGE_VALF) {
			// no valid path
			break;
		}

		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				if (x == 0 && y == 0) continue;
				// XXX: doing search in all 8 directions results in some
				//      diagonally intraversable tiles being marked as
				//      traversable here, need either directional traversal
				//      info to check that it's a valid path,
				//      or coooould just check that tiles adjacent to the
				//      diagonal are also traversable...
				//      would be a bit of a hack, but having diagonal
				//      distances is an asthetic choice to avoid enemies
				//      taking awkward clumsy routes in wide open spaces,
				//      falling back to 4-direction pathing would be fine
				//      in tight spaces and corners (maybe even preferable?)
				//
				//      idk, undecided, this works for now
				if (abs(x) == abs(y)) { continue; }

				Coord coord = {current.first + x, current.second + y};

				if (distances.valid(coord)
					&& !visited.count(coord)
					&& mask.get(coord))
				{
					//int  dist = distances[current.first][current.second] + 1;
					//int& other = distances[coord.first][coord.second];
					float foo = (abs(x) == abs(y))? 1.41421356f : 1.;
					float dist  = distances.get(current) + foo;
					float other = distances.get(coord);

					if (dist < other) {
						distances.set(coord, dist);
					}

					unvisited.insert(coord);
				}
			}
		}

		visited.insert(current);

		if (distances.get(current) > maxDist) {
			maxDist = distances.get(current);
			maxCoord = current;
		}
	}

	return maxCoord;
}

template <int X, int Y>
static void generateHotpathMap(array2D<int, X, Y>& arr,
                               array2D<float, X, Y>& distances,
                               array2D<bool, X, Y>& mask,
                               Coord start,
                               int target)
{
	std::set<Coord> nodes;
	//std::vector<Coord> unvisited;

	for (int x = 0; x < X; x++) {
		for (int y = 0; y < Y; y++) {
			if (arr.get(x, y) == target) {
				nodes.insert({x, y});
			}
		}
	}

	distances.clear(HUGE_VALF);
	for (auto& c : nodes) {
		generateDijkstraMap(distances, mask, c);
	}
}

// hmmmmmmmmmm
template <int X, int Y>
static 
std::pair<
	std::map<wfcGenerator::Array::Coord, wfcGenerator::Array>,
	std::map<wfcGenerator::Array::Coord, wfcGenerator::Array::Coord>>
generateOmnidijkstra(array2D<bool, X, Y>& mask)
{
	std::map<wfcGenerator::Array::Coord, wfcGenerator::Array> ret;
	std::map<wfcGenerator::Array::Coord, wfcGenerator::Array::Coord> maxes;

	std::set<Coord> nodes;
	//std::vector<Coord> unvisited;

	for (int x = 0; x < X; x++) {
		for (int y = 0; y < Y; y++) {
			if (mask.get(x, y)) {
				Coord c = {x, y};
				ret[c].clear(HUGE_VALF);

				Coord max = generateDijkstraMap(ret[c], mask, c);
				maxes[c] = max;
			}
		}
	}

	return {ret, maxes};
}

std::vector<wfcGenerator::Coord>
wfcGenerator::pathfind(glm::vec3 a, glm::vec3 b) {
	std::vector<wfcGenerator::Coord> ret;

	return ret;
}

glm::vec3
wfcGenerator::pathfindDirection(glm::vec3 a, glm::vec3 b) {
	auto ca = positionToCoord(a);
	auto cb = positionToCoord(b);

	auto pb = getCoordMap(cb);

	if (!pb) {
		return glm::vec3(0);
	}

	glm::vec3 ret;
	float score = HUGE_VALF;
	Array::Coord bestDir = {0, 0};

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			if (x == 0 && y == 0) continue;

			Array::Coord c = {ca.first + x, ca.second + y};
			float dist = pb->valid(c)? pb->get(c) : HUGE_VALF;

			if (dist < score) {
				score = dist;
				bestDir = {x, y};
			}
		}
	}

	if (bestDir != Array::Coord {0, 0}) {
		glm::vec2 foo = glm::normalize(glm::vec2(bestDir.first, bestDir.second));
		return glm::vec3(foo.x, 0, foo.y);

	} else {
		return glm::vec3(0);
	}
}

glm::vec3
wfcGenerator::pathfindAway(glm::vec3 a, glm::vec3 b) {
	auto ca = positionToCoord(a);
	auto cb = positionToCoord(b);

	auto pb = getCoordMap(cb);

	if (!pb) {
		return glm::vec3(0);
	}

	glm::vec3 ret;
	float score = 0;
	Array::Coord bestDir = {0, 0};

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			if (x == 0 && y == 0) continue;

			Array::Coord c = {ca.first + x, ca.second + y};
			float dist = pb->valid(c)? pb->get(c) : HUGE_VALF;

			if (dist != HUGE_VALF && dist > score) {
				score = dist;
				bestDir = {x, y};
			}
		}
	}

	if (bestDir != Array::Coord {0, 0}) {
		glm::vec2 foo = glm::normalize(glm::vec2(bestDir.first, bestDir.second));
		return glm::vec3(foo.x, 0, foo.y);

	} else {
		return glm::vec3(0);
	}
}

void wfcGenerator::clear(void) {
	mapdata.clear();
	traversableMask.clear();
	generatedMask.clear();
	placedMask.clear();

	omnidijkstra.clear();
	omniGenerated.clear();
}

void wfcGenerator::clearCaches(void) {
	omniGenerated.clear();
}

wfcGenerator::Array *wfcGenerator::getCoordMap(Array::Coord position) {
	if (regenerateCoordMaps(position)) {
		return &omnidijkstra[position];
	}

	return nullptr;
}

void wfcGenerator::regenerateMaps(void) {
	generateHotpathMap(mapdata, hotpathDistance, traversableMask, entry, 0xe1);
	//auto [omni, maxes] = generateOmnidijkstra(traversableMask);
	//omnidijkstra = omni;
	//maxDistances = maxes;
	placedMask = traversableMask;
}

bool wfcGenerator::regenerateCoordMaps(Array::Coord position) {
	if (omniGenerated.get(position)) {
		// if the map is already generated, nothing to do
		return true;
	}

	if (traversableMask.get(position)) {
		auto& om = omnidijkstra[position];
		om.clear(HUGE_VALF);

		Array::Coord max = generateDijkstraMap(om, traversableMask, position);
		maxDistances[position] = max;

		omniGenerated.set(position, true);
		fprintf(stderr, "regenerated: (%d, %d)\n", position.first, position.second);
		return true;
	}

	return false;
}

bool wfcGenerator::placeObject(Coord position, Coord dimensions) {
	// TODO:
	return true;
}

bool wfcGenerator::placeObjectRandomly(Coord dimensions) {
	// TODO:
	return true;
}

wfcGenerator::Array::Coord wfcGenerator::findNearest(
		WFCSolver<StateDef, genwidth, genheight>& wfcgrid,
		Array::Coord position,
		std::string tag)
{
	auto it = spec->tags.find(tag);

	if (it == spec->tags.end()) {
		return {-1, -1};
	}

	Array::Coord ret = {-1, -1};
	int mindist = INT_MAX;
	auto& tagset = it->second;

	// naive brute force... honestly should be good enough, there are better
	// ways to do this for sure, but N is small and this will only be run
	// a few times each generation for placing stairs and what not,
	// need to get this done quickly
	for (int x = 0; x < genwidth; x++) {
		for (int y = 0; y < genheight; y++) {
			int dist = abs(x - position.first) + abs(y - position.second);

			if (dist >= mindist) {
				// already found one closer
				continue;
			}

			auto& tile = wfcgrid.gridState.tiles[y*genwidth + x];

			for (auto& em : tile) {
				if (em && tagset.count(em)) {
					mindist = dist;
					ret = {x, y};
				}
			}
		}
	}

	return ret;
}

gameObject::ptr wfcGenerator::genCell(int x, int y, int z) {
	gameObject::ptr ret = std::make_shared<gameObject>();
	//WfcImpl *wfcgrid = getSector({x, y, z});

	bool valid = true;
	unsigned attempts = 0;
	std::random_device rd;
	std::mt19937 g(rd());

	using bspimp = bsp::bspGen<genwidth, genheight>;
	srand(time(NULL));

retry:
	WFCSolver<StateDef, genwidth, genheight> wfcgrid(spec->stateClass);
	bspimp mapskel;

	clear();
	mapskel.genSplits(4);
	mapskel.connectNodes(mapskel.root, mapdata, 20);

	auto leaves = mapskel.getLeafCenters();
	std::shuffle(leaves.begin(), leaves.end(), g);

	entry = leaves[0];

	//mapdata.set(entry.first, entry.second, 0xe0);
	mapdata.floodfill(entry.first, entry.second, 0xff, 0xe0);
	std::cerr << "did floodfill" << std::endl;
	mapdata.clearExcept(0xe0, 0x00);

	bool connects = false;

	for (unsigned i = 1; i < leaves.size(); i++) {
		auto& leaf = leaves[i];
		auto dist = [] (auto& a, auto& b) {
			return abs(a.first - b.first) + abs(a.second - b.second);
		};

		if (mapdata.get(leaf.first, leaf.second) == 0xe0) {
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

	markShortestPath(mapdata, entry, exitpoint, 0xe1);

	for (size_t bx = 0; bx < genwidth; bx++) {
		for (size_t by = 0; by < genheight; by++) {
			if (mapdata.get(bx, by)) {
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

	std::cerr << "Set empty perimeter" << std::endl;

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
		//static std::mutex mtx;
		//std::lock_guard g(mtx);

		for (size_t gx = 0; gx < genwidth - 1; gx++) {
			for (size_t gy = 0; gy < genheight - 1; gy++) {
				auto& tile = wfcgrid.gridState.tiles[gy*genwidth + gx];

				for (auto& em : tile) {
					if (em) {
						std::string objname = std::string("tile")
							+ "[" + std::to_string(gx) + "]" 
							+ "[" + std::to_string(gy) + "]";

						// keep track of what tiles actually collapsed to
						// traversable tiles, BSP doesn't reflect the final
						// output
						if (em) {
							generatedMask.set(gx, gy, true);
						}

						if (spec->tags["traversable"].count(em)) {
							traversableMask.set(gx, gy, true);
						}

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

	regenerateMaps();
	//generateHotpathMap(mapdata, hotpathDistance, generatedMask, entry, 0xe1);

	gameObject::ptr entryObj = std::make_shared<gameObject>();
	gameObject::ptr exitObj  = std::make_shared<gameObject>();

	entry     = findNearest(wfcgrid, entry, "replaceable");
	exitpoint = findNearest(wfcgrid, exitpoint, "replaceable");

	auto replaceTile = [&] (Array::Coord p, gameObject::ptr obj) {
		std::string objname = std::string("tile")
			+ "[" + std::to_string(p.first) + "]"
			+ "[" + std::to_string(p.second) + "]";

		auto ptr = ret->getNode(objname);
		auto rot = ptr->getNode("model");
		obj->setTransform((TRS) {
			.position = ptr->getTransformTRS().position,
			.rotation = rot->getTransformTRS().rotation
		});
		setNode(objname, ret, entryObj);
	};

	replaceTile(entry, entryObj);
	replaceTile(exitpoint, exitObj);

	/*
	entryObj->setTransform({.position = {4*entry.first, 0, 4*entry.second} });
	exitObj->setTransform({.position = {4*exitpoint.first, 0, 4*exitpoint.second} });
	*/

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
#if 0
	static bool food = false;
#endif

	// XXX: man I really have to clean this up at some point, ugh
	if (!havePhysics) {
		game->phys->addStaticModels(nullptr,
		                            root,
		                            root->getTransformTRS(),
		                            mapobjs,
		                            "collidable");
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
