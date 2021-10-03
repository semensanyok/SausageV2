#include "Test1.h"
#include "GLMHelpersTest.h"
#include "AssetUtilsTest.h"
#include "FontManagerTest.h"
#include "RendererContextManagerTest.h"

#undef main

int main()
{
	//Test1::run();
	//GLMHelpersTest::run();
	//AssetUtilsTest::run();
	//RendererContextManagerTest::run();
	FontManagerTest().run();
	return 0;
}