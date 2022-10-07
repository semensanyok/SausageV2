#pragma once

#include "sausage.h"
#include "Logging.h"

using namespace std;

class SausageHashable {
public:
  virtual size_t Hash() = 0;
  virtual ~SausageHashable() {};
  size_t operator()(SausageHashable& s) const
  {
    return s.Hash();
  }
};

class SausageUserPointer {
public:
  virtual ~SausageUserPointer() {};
};

class SausageSystem {
public:
  virtual ~SausageSystem() {};
};
