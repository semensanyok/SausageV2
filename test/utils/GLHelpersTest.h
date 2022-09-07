#pragma once

#include <sausage.h>
#include <GLHelpers.h>
#include <assert.h>

using namespace std;
using namespace glm;

class AssetUtilsTest {
public:
  static void run() {
    assert(GetPadTo16BytesNumOfBytes(sizeof(vec4)) == 0);
    assert(GetPadTo16BytesNumOfBytes(sizeof(int)) == 12);
    assert(GetPadTo16BytesNumOfBytes(sizeof(byte)) == 15);
    assert(GetPadTo16BytesNumOfBytes(sizeof(byte)) == 15);
  };
};
