#include "ThreadSafeNumberPool.h"

unsigned int ThreadSafeNumberPool::ObtainNumber()
{
  lock_guard l(mtx);
  // TODO: return -1 if failed?
  assert(allocated < max_number);
  if (!released_slots.empty()) {
    auto res = released_slots.top();
    released_slots.pop();
    return res;
  }
  return allocated++;
}

void ThreadSafeNumberPool::ReleaseNumber(unsigned int number)
{
  lock_guard l(mtx);
  assert(number > 0);
  assert(number < max_number);
  if (number == allocated - 1) {
    allocated--;
  }
  else {
    released_slots.push(number);
  }
}

void ThreadSafeNumberPool::Reset() {
  lock_guard l(mtx);
  allocated = 0;
  released_slots = {};
}
