#pragma once
#include "sausage.h"

using namespace std;

template <typename T, typename C>
class ThreadSafeSet {
  mutex mtx;
  condition_variable push_event;

public:
  set<T> container;
  ThreadSafeSet() : container{ set<T>(C) } {}
  ~ThreadSafeSet() {}
  set<T>::iterator PopInRange(
    const T& min_inclusive,
    const T& max_inclusive
  );
  set<T>::iterator End();;
  void Push(const T& element);
};
