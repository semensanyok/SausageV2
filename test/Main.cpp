#include "Test1.h"
#include "GLMHelpersTest.h"
#include "AssetUtilsTest.h"
#include "UIFontManagerTest.h"
#include "3DFontManagerTest.h"
#include "RendererContextManagerTest.h"
#include "GtkGLOverlayTest.h"
#include "ScreenOverlayManagerTest.h"

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
  ScreenOverlayManagerTest().run();
  return 0;
}
