#include "ThreadSafeQueue.h"

template<typename T>
queue<T> ThreadSafeQueue<T>::PopAll()
{
  std::lock_guard<std::mutex> mlock(mtx);
  auto res = queue<T>(container);
  container = queue<T>();
  return res;
}

template<typename T>
queue<T> ThreadSafeQueue<T>::WaitPopAll(bool& quit)
{
  std::unique_lock<std::mutex> mlock(mtx);
  while (container.empty() && !quit)
  {
    push_event.wait(mlock);
  }
  auto res = queue<T>(container);
  container = queue<T>();
  return res;
}

template<typename T>
T ThreadSafeQueue<T>::WaitPop(bool& quit)
{
  std::unique_lock<std::mutex> mlock(mtx);
  while (container.empty() && !quit)
  {
    push_event.wait(mlock);
  }
  auto element = container.front();
  container.pop();
  return element;
}

template<typename T>
void ThreadSafeQueue<T>::Push(const T& element)
{
  std::unique_lock<std::mutex> mlock(mtx);
  container.push(element);
  mlock.unlock();
  push_event.notify_all();
}
