#pragma once

#include "sausage.h"
#include "Logging.h"

using namespace std;

class SausageHashable {
public:
  virtual size_t Hash() = 0;
  virtual ~SausageHashable() {};
};

class SausageUserPointer {
public:
  virtual ~SausageUserPointer() {};
};

class SausageSystem {
public:
  virtual ~SausageSystem() {};
};
