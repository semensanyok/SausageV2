#pragma once

#include <sausage.h>
#include <GLMHelpers.h>
#include <assert.h>

using namespace std;
using namespace glm;

class GLMHelpersTest {
public:
	static void run() {
		auto eps = 1e-4;

		auto m1 = mat4(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		auto m2 = mat4(0.99999, eps, eps, 1e-5,
			eps, 1, eps, eps,
			eps, eps, 1, eps,
			eps, eps, eps, 1);
		auto m3 = mat4(0.99997, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		auto m4 = mat4(0.9997, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		auto m5 = mat4(0, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);


		assert(IsIdentity(m1));
		assert(IsIdentity(m2));
		assert(IsIdentity(m3));
		assert(!IsIdentity(m4));
		assert(!IsIdentity(m5));
	};
};
