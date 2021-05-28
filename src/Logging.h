#pragma once

#include "sausage.h"
#include "OpenGLHelpers.h"
#include "utils/ThreadSafeQueue.h"

using namespace std;

void LOG(const string& s);

thread LogIO(bool& quit);

void WriteShaderMsgsToLogFile();
