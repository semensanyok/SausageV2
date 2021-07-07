#pragma once

#include "sausage.h"
#include "utils/ThreadSafeQueue.h"
#include "OpenGLHelpers.h"

using namespace std;

void LOG(const string& s);

namespace Sausage {
	void LogIO();

	void WriteShaderMsgsToLogFile();
}