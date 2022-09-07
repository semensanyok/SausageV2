#pragma once

#include "sausage.h"

using namespace std;

constexpr size_t GetPadTo16BytesNumOfBytes(size_t size_of_type) {
	return 16 - size_of_type % 16;
}