#include "Test1.h"
#include "GLMHelpersTest.h"
#include "AssetUtilsTest.h"
#include "UIFontManagerTest.h"
#include "3DFontManagerTest.h"
#include "RendererContextManagerTest.h"
#include "GtkGLOverlayTest.h"
#include "ScreenOverlayManagerTest.h"
#include "world_gen/TerrainTest.h"
#include "GLHelpersTest.h"

#undef main

int main()
{
	//Test1::run();
	//GLMHelpersTest::run();
	//AssetUtilsTest::run();
	//RendererContextManagerTest::run();
	//UIFontManagerTest().run();
  //FontManagerTest3D().run();
  //GtkGLOverlayTest().run();

  //ScreenOverlayManagerTest().run();
  TerrainTest().run();

  //GLHelpersTest().run();
  return 0;
}
