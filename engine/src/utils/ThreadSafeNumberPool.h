#pragma once

#include "sausage.h"
#include "Macros.h"

using namespace std;

class ThreadSafeNumberPool {
  const unsigned int max_number;
  unsigned int allocated;
  unsigned int min_released_slot = INT_MAX;
  stack<unsigned int> released_slots;
  mutex mtx;
public:
  ThreadSafeNumberPool(unsigned int max_number = UINT_MAX) :
    max_number{ max_number },
    allocated{ 0 } {
  };
  unsigned int ObtainNumber();
  void ReleaseNumber(unsigned int number);
  void Reset();
};
