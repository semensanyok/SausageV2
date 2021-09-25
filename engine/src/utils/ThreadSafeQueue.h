#pragma once
#include "sausage.h"

using namespace std;

template <typename T>
class ThreadSafeQueue {
	mutex log_mutex;
	condition_variable is_log_event;
	bool end;

public:
	queue<T> container;
	// ThreadSafeQueue();
	// ~ThreadSafeQueue();
	// queue<T> PopAll();
	// queue<T> WaitPopAll(bool& quit);
	// T WaitPop(bool& quit);

	//void Push(const T& message);

	inline ThreadSafeQueue() : container{ queue<T>() } {}

	inline ~ThreadSafeQueue() {}

	inline queue<T> PopAll()
	{
		std::lock_guard<std::mutex> mlock(log_mutex);
		auto res = queue<T>(container);
		container = queue<T>();
		return res;
	}

	inline queue<T> WaitPopAll(bool& quit)
	{
		std::unique_lock<std::mutex> mlock(log_mutex);
		while (container.empty() && !quit)
		{
			is_log_event.wait(mlock);
		}
		auto res = queue<T>(container);
		container = queue<T>();
		return res;
	}

	inline T WaitPop(bool& quit)
	{
		std::unique_lock<std::mutex> mlock(log_mutex);
		while (container.empty() && !quit)
		{
			is_log_event.wait(mlock);
		}
		auto message = container.front();
		container.pop();
		return message;
	}

	inline void Push(const T& message)
	{
		std::unique_lock<std::mutex> mlock(log_mutex);
		container.push(message);
		mlock.unlock();
		is_log_event.notify_all();
	}

};
