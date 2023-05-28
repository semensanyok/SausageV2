#pragma once

#include "sausage.h"

using namespace std;

template <typename T>
class ThreadSafeVector {
public:
  // exposed lock for safe iteration (func param was considered worse)
  mutex mtx;
  vector<T> container;
  inline ThreadSafeVector() : container{ } {}
  inline ThreadSafeVector(unsigned int size) : container{ vector<T>(size) } {}
  inline ~ThreadSafeVector() {}
  inline void EraseThreadSafe(unsigned int& index)
  {
    Erase(index);
  }
  inline void Erase(unsigned int& index)
  {
    lock_guard<mutex> mlock(mtx);
    if (index > container.size() - 1) {
      return;
    }
    container.erase(advance(container.begin(), index));
  }

  inline unsigned int PushThreadSafe(const T& element)
  {
    return Push(element);
  }
  inline unsigned int Push(const T& element)
  {
    lock_guard<mutex> mlock(mtx);
    container.push_back(element);
    return container.size() - 1;
  }
};
