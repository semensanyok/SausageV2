#pragma once

#include "sausage.h"
#include "Macros.h"

using namespace std;

// refactor to not keep array
class ThreadSafeNumberPool {
  const unsigned int max_number;
  stack<unsigned int> numbers;
  mutex mtx;
public:
  ThreadSafeNumberPool(unsigned int max_number) : max_number{ max_number } {
    for (int i = max_number; i >= 0; i--) {
      numbers.push(i);
    }
  };
  unsigned int ObtainNumber();
  void ReleaseNumber(unsigned int number);
};
