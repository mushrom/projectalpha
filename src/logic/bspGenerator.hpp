#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <assert.h>

#include "array2D.hpp"

namespace bsp {

struct bspNode {
	~bspNode() {
		if (left)  delete left;
		if (right) delete right;
	}

	size_t relX;
	size_t relY;

	size_t width;
	size_t height;

	bspNode *left  = nullptr;
	bspNode *right = nullptr;
};

static inline bool hasSplit(bspNode *node) {
	// should have both or neither
	assert((node->left && node->right) || (!node->left && !node->right));
	return node->left && node->right;
}

static inline bool canSplitX(bspNode *node) {
	return node->width > 1;
}

static inline bool canSplitY(bspNode *node) {
	return node->height > 1;
}

static inline void splitX(bspNode *node, size_t x) {
	// no valid split
	if (!canSplitX(node) || x >= node->width) return;

	node->relX = x;
	node->relY = 0;

	node->left = new bspNode;
	node->right = new bspNode;

	node->left->width = x;
	node->right->width = node->width - x;
	node->left->height = node->right->height = node->height;
}

static inline void splitY(bspNode *node, size_t y) {
	// no valid split
	if (!canSplitY(node) || y >= node->height - 1) return;

	node->relX = 0;
	node->relY = y;

	node->left = new bspNode;
	node->right = new bspNode;

	node->left->height = y;
	node->right->height = node->height - y;
	node->left->width = node->right->width = node->width;
}

template <size_t X, size_t Y>
class bspGen {
	public:
		using Coord = std::pair<int, int>;
		using Array = array2D<int, X, Y>;
		bspNode *root = nullptr;

		bspGen() {
			reset();
		}

		~bspGen() {
			delete root;
		}

		void reset(void) {
			if (root) delete root;

			root = new bspNode;
			root->width  = X;
			root->height = Y;
		}

		bspNode *getRandomLeaf(void) {
			auto ptr = root;

			while (ptr->left && ptr->right) {
				ptr = (rand()&1)? ptr->left : ptr->right;
			}

			return ptr;
		}

		void randomSplit(void) {
			auto node = getRandomLeaf();

			if (canSplitX(node) && (rand()&1)) {
				//splitX(node, 1 + rand()%(node->width - 1));
				splitX(node, node->width / 2);
				//splitX(node, node->width * 0.71);

			} else if (canSplitY(node)) {
				//splitY(node, 1 + rand()%(node->height - 1));
				splitY(node, node->height / 2);
				//splitY(node, node->height * 0.71);
			}
		}

		void genSplits(size_t n) {
			for (size_t i = 0; i < n; i++) {
				randomSplit();
			}
		}

		size_t dumpGreyscale(bspNode *node,
		                     Array& arr,
		                     size_t idx,
		                     size_t x,
		                     size_t y)
		{
			if (!node) return idx;

			if (hasSplit(node)) {
				size_t next = dumpGreyscale(node->left, arr, idx, x, y);
				return dumpGreyscale(node->right, arr, next, x + node->relX, y + node->relY);

			} else {
				for (size_t ix = 0; ix < node->width /*- 1*/; ix++) {
					for (size_t iy = 0; iy < node->height /*- 1*/; iy++) {
						//px[(y + iy)*X + (x + ix)] = idx;
						arr.set(x+ix, y+iy, idx);
					}
				}

				//px[(y + node->height/2)*X + (x + node->width/2)] = idx;

				return idx + 16;
			}
		}

		void getCenterIter(bspNode *node, std::vector<Coord>& vec, size_t x, size_t y) {
			if (hasSplit(node)) {
				getCenterIter(node->left,  vec, x, y);
				getCenterIter(node->right, vec, x + node->relX, y + node->relY);

			} else {
				vec.push_back({x + node->width/2, y + node->height/2});
			}
		}

		std::vector<Coord> getLeafCenters(void) {
			std::vector<Coord> ret;
			getCenterIter(root, ret, 0, 0);
			return ret;
		}

		void connectNodes(bspNode *node, Array& arr, size_t maxDist) {
			auto nodes = getLeafCenters();

			for (auto& em : nodes) {
				std::vector<std::pair<size_t, size_t>> distances;
				//unsigned connections = 1 + rand()&3;
				//unsigned connections = 3 + (rand()&1);
				unsigned connections = 2 + (rand()&1);
				//unsigned connections = 1 + rand()&3;
				//unsigned connections = 2 + rand()&3;

				for (size_t i = 0; i < nodes.size(); i++) {
					int x = abs(em.first  - nodes[i].first);
					int y = abs(em.second - nodes[i].second);

					distances.push_back({i, x + y});
				}

				using distEnt = std::pair<size_t, size_t>;
				std::sort(distances.begin(), distances.end(),
					[&](distEnt& a, distEnt& b) {
						return a.second < b.second;
					});

				for (unsigned i = 1; i < connections + 1 && i < nodes.size(); i++) {
					//if (distances[i].second > maxDist)
					//	continue;

					auto& coord = nodes[distances[i].first];

					auto incx = [&] () {
						for (int x = em.first;; x += (em.first < coord.first)? 1 : -1) {
							//px[em.second*X + x] = 0xff;
							arr.set(x, em.second, 0xff);

							if (x == coord.first)
								break;
						}
					};

					auto incy = [&] () {
						for (int y = em.second;; y += (em.second < coord.second)? 1 : -1) {
							//px[y*X + em.first] = 0xff;
							arr.set(em.first, y, 0xff);

							if (y == coord.second)
								break;
						}
					};

					if (rand()&1) {
						incy();
						incx();

					} else {
						incx();
						incy();
					}
				}
			}
		}
};

// namespace bspgen
}
