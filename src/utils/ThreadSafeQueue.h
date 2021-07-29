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
	ThreadSafeQueue() : container{queue<T>()} {};
	~ThreadSafeQueue() {};
	queue<T> PopAll()
	{
		std::lock_guard<std::mutex> mlock(log_mutex);
		auto res = queue<T>(container);
		container = queue<T>();
		return res;
	}
	queue<T> WaitPopAll(bool& quit)
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
	T WaitPop(bool &quit)
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

	void Push(const T& message)
	{
		std::unique_lock<std::mutex> mlock(log_mutex);
		container.push(message);
		mlock.unlock();
		is_log_event.notify_all();
	}

};
