#pragma once
#include "sausage.h"

using namespace std;

template <typename T>
class ThreadSafeQueue {
  mutex mtx;
  condition_variable push_event;

public:
  queue<T> container;
  ThreadSafeQueue() : container{ queue<T>() } {}
  ~ThreadSafeQueue() {}
  queue<T> PopAll();
  queue<T> WaitPopAll(bool& quit);
  T WaitPop(bool& quit);
  void Push(const T& element);
};
