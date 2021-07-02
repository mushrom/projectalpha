#pragma once
#include <array>

template <typename T, int X, int Y>
class array2D {
	public:
		using Coord = std::pair<int, int>;

		std::array<T, X*Y> data;

		array2D() { clear(); }

		constexpr int width()  const { return X; };
		constexpr int height() const { return Y; };

		array2D& operator=(const array2D& arr) {
			for (int x = 0; x < X; x++) {
				for (int y = 0; y < Y; y++) {
					set(x, y, arr.get(x, y));
				}
			}

			return *this;
		}

		bool valid(int x, int y) const {
			return x >= 0 && y >= 0 && x < X && y < Y;
		}

		bool valid(Coord c) const {
			return valid(c.first, c.second);
		}

		size_t index(int x, int y) const {
			return y*X + x;
		}

		T get(int x, int y) const {
			if (valid(x, y)) {
				return data[index(x, y)];
			}

			return T();
		}

		T get(Coord c) const {
			return get(c.first, c.second);
		}

		void set(int x, int y, T value) {
			if (valid(x, y)) {
				data[index(x, y)] = value;
			}
		}

		void set(Coord c, T value) {
			set(c.first, c.second, value);
		}

		void clear(T value = 0) {
			for (int x = 0; x < X; x++) {
				for (int y = 0; y < Y; y++) {
					set(x, y, value);
				}
			}
		}

		void floodfill(int x, int y, T target, T replace) {
			if (!valid(x, y)) return;

			if (get(x, y) == target) {
				set(x, y, replace);
				//cur = replace;

				floodfill(x + 1, y, target, replace);
				floodfill(x - 1, y, target, replace);
				floodfill(x, y + 1, target, replace);
				floodfill(x, y - 1, target, replace);
			}
		}

		void clearExcept(T target, T replace) {
			for (int x = 0; x < X; x++) {
				for (int y = 0; y < Y; y++) {
					if (get(x, y) != target) {
						set(x, y, replace);
					}
				}
			}
		}
};
