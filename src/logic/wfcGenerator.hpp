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

#include <thread>
#include <map>
#include <memory>
#include <utility>

using namespace grendx;
using namespace grendx::ecs;
using namespace wfc;

using StateDef = stateDefinition2D<staticStateSet<256>>;

class wfcSpec {
	public:
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

class wfcGenerator : public worldGenerator {
	public:
		static constexpr int genwidth  = 32;
		static constexpr int genheight = 32;

		wfcGenerator(gameMain *game, std::string spec, unsigned seed = 0xcafebabe);
		virtual ~wfcGenerator();
		void generate(gameMain *game, std::vector<glm::vec3> entries);
		virtual void setPosition(gameMain *game, glm::vec3 position);

		using WfcImpl = WFCSolver<StateDef, genwidth, genheight>;
		using WfcPtr = std::unique_ptr<WfcImpl>;
		using Coord  = std::tuple<int, int, int>;

	private:
		std::map<Coord, WfcPtr> sectors;
		std::unique_ptr<wfcSpec> spec;

		gameObject::ptr genCell(int x, int y, int z);
		void parseJson(gameMain *game, std::string filename);
		std::future<bool> genjob;
		gameObject::ptr returnValue;

		bool havePhysics = false;
		std::vector<physicsObject::ptr> mapobjs;
};

// XXX: global variable, TODO: something else
//      (could just put it in the landscapeGenerator contructor)
inline material::ptr   landscapeMaterial;
inline gameObject::ptr landscapeNodes = std::make_shared<gameObject>();
inline gameModel::ptr  treeNode;
