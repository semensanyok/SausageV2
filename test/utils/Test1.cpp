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
	static void run() {
		mat4 a;
		quat b = quat(0,0,0,0);
		vec4 c = vec4(0,0,0,0);
		printT(b);
		printT(c);
	};
};