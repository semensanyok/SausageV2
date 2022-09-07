#include "NumberPool.h"

unsigned int NumberPool::ObtainNumber()
{
  DEBUG_ASSERT(!numbers.empty());
  auto elem = numbers.top();
  numbers.pop();
  return elem;
}

void NumberPool::ReleaseNumber(unsigned int number)
{
  DEBUG_ASSERT(number > 0);
  DEBUG_ASSERT(number < max_number);
  numbers.push(number);
}
