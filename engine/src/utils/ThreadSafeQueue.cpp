#include "ThreadSafeQueue.h"

//template<typename T>
//inline ThreadSafeQueue<T>::ThreadSafeQueue() : container{ queue<T>() } {}
//
//template<typename T>
//inline ThreadSafeQueue<T>::~ThreadSafeQueue() {}
//
//template<typename T>
//inline queue<T> ThreadSafeQueue<T>::PopAll()
//{
//	std::lock_guard<std::mutex> mlock(log_mutex);
//	auto res = queue<T>(container);
//	container = queue<T>();
//	return res;
//}
//
//template<typename T>
//inline queue<T> ThreadSafeQueue<T>::WaitPopAll(bool& quit)
//{
//	std::unique_lock<std::mutex> mlock(log_mutex);
//	while (container.empty() && !quit)
//	{
//		is_log_event.wait(mlock);
//	}
//	auto res = queue<T>(container);
//	container = queue<T>();
//	return res;
//}
//
//template<typename T>
//inline T ThreadSafeQueue<T>::WaitPop(bool& quit)
//{
//	std::unique_lock<std::mutex> mlock(log_mutex);
//	while (container.empty() && !quit)
//	{
//		is_log_event.wait(mlock);
//	}
//	auto message = container.front();
//	container.pop();
//	return message;
//}
//
//template<typename T>
//inline void ThreadSafeQueue<T>::Push(const T& message)
//{
//	std::unique_lock<std::mutex> mlock(log_mutex);
//	container.push(message);
//	mlock.unlock();
//	is_log_event.notify_all();
//}
