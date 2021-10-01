#pragma once

#include <sausage.h>
#include <AssetUtils.h>
#include <assert.h>

using namespace std;
using namespace glm;

class AssetUtilsTest {
public:
	static void run() {
		auto p = GetFontPath("arial.ttf");
		assert(strcmp(p.c_str(), "..\\assets\\fonts\\arial.ttf") == 0);
	};
};