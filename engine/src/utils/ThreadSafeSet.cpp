#include "ThreadSafeSet.h"

template<typename T, typename C>
set<T>::iterator ThreadSafeSet<T, C>::PopInRange(
    const T& min_inclusive,
    const T& max_inclusive
)
{
  std::unique_lock<std::mutex> mlock(mtx);
  auto element = container.lower_bound(min_inclusive);
  if (element != container.end() && element <= max_inclusive) {
    container.erase(element);
  }
  return element;
}

template<typename T, typename C>
set<T>::iterator ThreadSafeSet<T, C>::End() {
  return container.end();
};

template<typename T, typename C>
void ThreadSafeSet<T, C>::Push(const T& element)
{
  std::unique_lock<std::mutex> mlock(mtx);
  container.push(element);
  mlock.unlock();
  push_event.notify_all();
}
