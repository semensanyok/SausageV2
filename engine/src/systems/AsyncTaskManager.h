#pragma once

#include "../sausage.h"
#include "../utils/ThreadSafeQueue.h"
#include "../Settings.h"

using namespace std;

class AsyncTaskManager {
	int MAX_THREADS = std::thread::hardware_concurrency();
	thread physics_thread;
	thread anim_thread;
	thread misc_thread;

	ThreadSafeQueue<pair<function<void()>, bool>> misc_tasks;
	ThreadSafeQueue<pair<function<void()>, bool>> physics_tasks;
	ThreadSafeQueue<pair<function<void()>, bool>> anim_tasks;
public:

	AsyncTaskManager() {};
	void Run();
	void SubmitPhysTask(function<void()> task, bool is_persistent);
	void SubmitMiscTask(function<void()> task, bool is_persistent);
	void SubmitAnimTask(function<void()> task, bool is_persistent);
	~AsyncTaskManager();
private:
	void RunPhysicsTasks(bool& quit);
	void RunMiscTasks(bool& quit);
	void RunAnimTasks(bool& quit);
};