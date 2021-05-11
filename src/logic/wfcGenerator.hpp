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
		wfcSpec(gameMain *game, std::string filename);
		void parseJson(gameMain *game, std::string filename);

		StateDef stateClass;
		std::vector<gameObject::ptr> models;

		std::map<std::string, size_t> nameToState;
		std::map<size_t, std::string> stateToName;
		std::map<std::string, std::set<size_t>> tags;
};

class wfcGenerator : public worldGenerator {
	public:
		static constexpr int genwidth  = 32;
		static constexpr int genheight = 32;

		wfcGenerator(gameMain *game, std::string spec, unsigned seed = 0xcafebabe);
		virtual ~wfcGenerator();
		virtual void setPosition(gameMain *game, glm::vec3 position);

		using WfcImpl = WFCSolver<StateDef, genwidth, genheight>;
		using WfcPtr = std::unique_ptr<WfcImpl>;
		using Coord  = std::tuple<int, int, int>;

	private:
		std::map<Coord, WfcPtr> sectors;
		std::unique_ptr<wfcSpec> spec;

		void generate(gameMain *game, std::vector<glm::vec3> entries);
		gameObject::ptr genCell(int x, int y, int z);
		void parseJson(gameMain *game, std::string filename);
		std::future<bool> genjob;
		gameObject::ptr returnValue;
};

// XXX: global variable, TODO: something else
//      (could just put it in the landscapeGenerator contructor)
inline material::ptr   landscapeMaterial;
inline gameObject::ptr landscapeNodes = std::make_shared<gameObject>();
inline gameModel::ptr  treeNode;
