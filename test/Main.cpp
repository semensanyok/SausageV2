#include "Test1.h"
#include "GLMHelpersTest.h"
#include <SystemsManager.h>

#undef main

int main()
{
	SystemsManager* m = new SystemsManager();
	m->InitSystems();
	Test1::run();
	GLMHelpersTest::run();
	return 0;
}