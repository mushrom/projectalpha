#pragma once

#include <grend/gameObject.hpp>
#include <grend/animation.hpp>
#include <grend/ecs/ecs.hpp>
#include <grend/ecs/collision.hpp>

#include <wfc-test/wfc.hpp>
#include <wfc-test/stateSet.hpp>
#include <wfc-test/stateDefinition2D.hpp>
#include <nlohmann/json.hpp>

#include "generatorEvent.hpp"
#include "array2D.hpp"

#include <thread>
#include <map>
#include <memory>
#include <utility>

using namespace grendx;
using namespace grendx::ecs;
using namespace wfc;

//using StateDef = stateDefinition2D<staticStateSet<512>>;
using StateDef = stateDefinition2D<dynamicStateSet>;

class wfcSpec {
	public:
		using ptr = std::shared_ptr<wfcSpec>;
		using weakptr = std::weak_ptr<wfcSpec>;

		using Name = std::pair<std::string, int>; /* name, rotation */

		wfcSpec(gameMain *game, std::string filename);
		void parseJson(gameMain *game, std::string filename);

		StateDef stateClass;
		std::map<std::string, gameObject::ptr> models;

		std::map<Name, unsigned> nameToState;
		std::map<unsigned, gameObject::ptr> stateModels;
		std::map<unsigned, Name> stateToName;
		std::map<std::string, std::set<unsigned>> tags;
};

// TODO: should base be renamed to worldState? might be more fitting than
//       generator, having a more general interface for pathfinding
//       and item placements etc
class wfcGenerator : public worldGenerator {
	public:
		static constexpr int genwidth  = 24;
		static constexpr int genheight = 24;

		wfcGenerator(gameMain *game, wfcSpec::ptr, unsigned seed = 0xcafebabe);
		virtual ~wfcGenerator();

		void generate(gameMain *game, std::vector<glm::vec3> entries);
		virtual void setPosition(gameMain *game, glm::vec3 position);

		using WfcImpl = WFCSolver<StateDef, genwidth, genheight>;
		using WfcPtr = std::unique_ptr<WfcImpl>;
		using Coord  = std::tuple<int, int, int>;
		using Array  = array2D<float, genwidth, genheight>;
		using ArrayInt = array2D<int, genwidth, genheight>;

		// some interface functions
		bool setTraversable(glm::vec3 p, bool traversable);
		/// find full path between two coordinates
		/// for 2D generation the Y component can be ignored
		std::vector<Coord> pathfind(glm::vec3 a, glm::vec3 b);

		/// normalized direction down the gradient, following this
		/// each iteration (frame) results in reaching the point 'b' in
		/// optimal time
		glm::vec3 pathfindDirection(glm::vec3 a, glm::vec3 b);

		/// normalized direction to run away from point 'b'
		glm::vec3 pathfindAway(glm::vec3 a, glm::vec3 b);

		//
		bool placeObject(Coord position, Coord dimensions);
		bool placeObjectRandomly(Coord dimensions);

		// these will probably be specific to this class, no need for 'Coord'...
		
		void clear(void);
		void regenerateMaps(void);
		bool regenerateCoordMaps(Array::Coord position);
		void clearCaches(void);
		Array *getCoordMap(Array::Coord position);
		Array::Coord findNearest(
				// TODO: should store WFC state in this class...
				WFCSolver<StateDef, genwidth, genheight>& wfcgrid,
				Array::Coord position,
				std::string tag);

		static inline Array::Coord positionToCoord(glm::vec3 pos) {
			return {int(pos.x/4 + 0.5), int(pos.z/4 + 0.5)};
		}

		array2D<int, genwidth, genheight> mapdata;
		ArrayInt::Coord entry, exitpoint;
		Array hotpathDistance;
		Array outbounds;

		// map of all traversable tiles
		array2D<bool, genwidth, genheight> traversableMask;
		// map of all generated (ie, not empty) tiles
		array2D<bool, genwidth, genheight> generatedMask;
		// map of tiles that are empty (out of bounds)
		array2D<bool, genwidth, genheight> outboundMask;
		// map of available places to place entities, is equivalent to
		// the traversable mask after generation
		array2D<bool, genwidth, genheight> placedMask;

		// generate maps as needed to spread out costs, essentially
		// working as a cache
		// TODO: changing world pathing state means clearing this whole
		//       array and generating a few maps, relatively expensive for
		//       something that might be as simple as opening a door...
		array2D<bool, genwidth, genheight> omniGenerated;
		// per-tile dijkstra map
		std::map<Array::Coord, Array> omnidijkstra;
		// per-tile dijkstra map, targeting fleeing from a point
		// rather than getting closer to it
		std::map<Array::Coord, Array> fleeMap;

		// for each dijkstra map, keep track of where the maximum distance
		// from the starting point is (useful for evasion)
		std::map<Array::Coord, Array::Coord> maxDistances;
		//array2D<Array*, genwidth, genheight> omnidijkstra;
		//std::array<Array*, genwidth*genheight>

		std::vector<physicsObject::ptr> mapobjs;
	private:
		std::map<Coord, WfcPtr> sectors;
		//std::unique_ptr<wfcSpec> spec;
		wfcSpec::ptr spec;

		gameObject::ptr genCell(int x, int y, int z);
		void parseJson(gameMain *game, std::string filename);
		std::future<bool> genjob;
		gameObject::ptr returnValue;

		// XXX
		bool havePhysics = false;
};

// XXX: global variable, TODO: something else
//      (could just put it in the landscapeGenerator contructor)
inline material::ptr   landscapeMaterial;
inline gameObject::ptr landscapeNodes = std::make_shared<gameObject>();
inline gameModel::ptr  treeNode;
