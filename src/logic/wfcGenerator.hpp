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

class wfcGenerator : public worldGenerator {
	public:
		wfcGenerator(gameMain *game, std::string spec, unsigned seed = 0xcafebabe);
		virtual ~wfcGenerator();
		virtual void setPosition(gameMain *game, glm::vec3 position);

		using StateDef = stateDefinition2D<staticStateSet<256>>;
		using WfcImpl = WFCSolver<StateDef, 10, 10>;
		using WfcPtr = std::unique_ptr<WfcImpl>;
		using Coord  = std::tuple<int, int, int>;

	private:
		StateDef stateClass;
		std::vector<gameObject::ptr> models;
		std::map<Coord, WfcPtr> sectors;

		WfcImpl *getSector(const Coord& coord);

		void generate(gameMain *game, glm::vec3 curpos, glm::vec3 lastpos);
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
