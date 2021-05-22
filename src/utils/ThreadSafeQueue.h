#pragma once
#include <mutex>
#include <queue>
#include <string>

class ThreadSafeQueue {
	ofstream logstream;
	queue<string> log;
	mutex log_mutex;
	condition_variable is_log_event;
	bool end;

public:
	ThreadSafeQueue() : log{queue<string>()} {};
	~ThreadSafeQueue() {};
	string pop(bool &quit)
	{
		std::unique_lock<std::mutex> mlock(log_mutex);
		while (log.empty() && !quit)
		{
			is_log_event.wait(mlock);
		}
		auto message = log.front();
		log.pop();
		return message;
	}

	void push(const string& message)
	{
		std::unique_lock<std::mutex> mlock(log_mutex);
		log.push(message);
		mlock.unlock();
		is_log_event.notify_one();
	}

};
