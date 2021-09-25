#include "GLMHelpers.h"

bool IsIdentity(mat4& m1) {
	auto d = m1 - mat4(1);
	auto eps = 1e-4;
	bool is_eq = true;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			is_eq &= (abs(d[i][j]) < eps);
		}
	}
	return is_eq;
}