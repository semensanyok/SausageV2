#pragma once
#include <RendererContextManager.h>
class RendererContextManagerTest {
public:
	static void run() {
		auto ctx = new RendererContextManager();
		ctx->InitContext();
		ctx->ClearContext();
	};
};