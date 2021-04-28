#pragma once

#include <grend/gameObject.hpp>
#include <grend/animation.hpp>
#include <grend/ecs/ecs.hpp>
#include <grend/ecs/collision.hpp>
#include <thread>

#include <wfc-test/static_flagset.hpp>
#include <wfc-test/wfc.hpp>
#include <nlohmann/json.hpp>

#include "generatorEvent.hpp"

using namespace grendx;
using namespace grendx::ecs;

// TODO: should be part of wfc stuff
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

class wfcGenerator : public worldGenerator {
	public:
		wfcGenerator(gameMain *game, std::string spec, unsigned seed = 0xcafebabe);
		virtual ~wfcGenerator();
		virtual void setPosition(gameMain *game, glm::vec3 position);

	private:
		statedef stateClass;
		std::vector<gameObject::ptr> models;

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
