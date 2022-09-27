#include "ThreadSafeNumberPool.h"

unsigned int ThreadSafeNumberPool::ObtainNumber()
{
  lock_guard(mtx);
  DEBUG_ASSERT(!numbers.empty());
  auto elem = numbers.top();
  numbers.pop();
  return elem;
}

void ThreadSafeNumberPool::ReleaseNumber(unsigned int number)
{
  lock_guard(mtx);
  DEBUG_ASSERT(number > 0);
  DEBUG_ASSERT(number < max_number);
  numbers.push(number);
}
