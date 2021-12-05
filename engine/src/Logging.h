﻿#pragma once

#include "sausage.h"
#include "ThreadSafeQueue.h"
#include "OpenGLHelpers.h"

using namespace std;

void LOG(const string& s,
         const std::source_location& location = std::source_location::current());

void LOG(const ostringstream& s,
         const std::source_location& location = std::source_location::current());

namespace Sausage {
	void LogIO();

	void WriteShaderMsgsToLogFile();
}
