#pragma once

#include "sausage.h"
#include "Macros.h"

using namespace std;

class ThreadSafeNumberPool {
  const unsigned int max_number;
  unsigned int allocated;
  stack<unsigned int> released_slots;
  mutex mtx;
public:
  ThreadSafeNumberPool(unsigned int max_number) :
    max_number{ max_number },
    allocated{ 0 } {
  };
  unsigned int ObtainNumber();
  void ReleaseNumber(unsigned int number);
  void Reset();
};
