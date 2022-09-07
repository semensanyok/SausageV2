#pragma once

#include "sausage.h"
#include "Macros.h"

using namespace std;

class NumberPool {
  const unsigned int max_number;
  stack<unsigned int> numbers;
public:
  NumberPool(unsigned int max_number) : max_number{ max_number } {
    for (unsigned int i = max_number; i > 0; i--) {
      numbers.push(i);
    }
  };
  unsigned int ObtainNumber();
  void ReleaseNumber(unsigned int number);
};
