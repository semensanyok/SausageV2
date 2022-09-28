#include "ThreadSafeNumberPool.h"
#include "ThreadSafeNumberPool.h"
#include "ThreadSafeNumberPool.h"

unsigned int ThreadSafeNumberPool::ObtainNumber()
{
  lock_guard(mtx);
  DEBUG_ASSERT(allocated < max_number);
  if (!released_slots.empty()) {
    return released_slots.pop();
  }
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

void ThreadSafeNumberPool::Reset() { lock_guard(mtx); allocated = 0; released_slots = {}; }
