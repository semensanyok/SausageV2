#include "sausage.h"

using namespace std;
using namespace glm;

class Test1 {
	template <typename T> static void printT(T a) {
		if (typeid(T) == typeid(quat)) {
			printf("quat");
		};
		if (typeid(T) == typeid(vec4)) {
			printf("vec");
		};
	}
public:
#define BLEND_FUNC(first, second, blend) typeid(first) == typeid(quat) ? slerp(first, second, blend) : mix(first, second, blend);
	static void run() {
		// mat4 a;
		quat b = quat(1, 0.5, 0, 0.5);
		quat b2 = quat(1, 0, 0.5, 0);
		auto c = vec3(1, 2, 3);
		auto c2 = vec3(2, 3, 4);
		// bool is_quat = typeid(b) == typeid(quat);
		float bl = 0.5;
		auto res = BLEND_FUNC(b, b2, bl);
		//quat res2 = BLEND_FUNC(c, c2, bl);
		// cout << res.x << endl;
		auto res3 = (quat*)_GetBlendAnim<quat>(&b, &b2, bl);
		auto res33 = slerp(b, b2, bl);
		//cout << to_string(res) << endl;
		auto res4 = (vec3*)_GetBlendAnim<vec3>(&c, &c2, bl);
		//cout << to_string(res4) << endl;

	};

#define BLEND_FUNC2(first, second, blend) typeid(first) == typeid(quat) ? slerp(first, second, blend) : mix(first, second, blend);
	template <typename T> static void* _GetBlendAnim(
		void* first,
		void* second,
		float blend
	) {
		if (typeid(T) == typeid(quat)) {
			auto q = slerp(*((quat*)first), *((quat*)second), blend);
			return &q;
		}
		auto v = mix(*((vec3*)first), *((vec3*)second), blend);
		return &v;
	}
};