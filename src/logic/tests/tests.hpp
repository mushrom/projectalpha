#pragma once

#include <grend/gameMain.hpp>
#include <logic/projalphaView.hpp>

namespace tests {

typedef bool (*testFunction)(gameMain *game, projalphaView::ptr view);
bool defaultTest(gameMain *game, projalphaView::ptr view);

// namespace tests
}
