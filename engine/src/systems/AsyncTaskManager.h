#pragma once

#include "sausage.h"
#include "ThreadSafeQueue.h"
#include "Settings.h"
#include "Structures.h"
#include "Interfaces.h"

using namespace std;

struct AsyncTask {
  function<void()> run;
  bool is_persistent;
};

class AsyncTaskManager : public SausageSystem {
	int MAX_THREADS = std::thread::hardware_concurrency();
    // not cast in stone, first iteration - made simple. dedicated threads for heavy tasks, and one thread for others.
    // for better work distribution consider:
    //  - having a pool of threads and pool of tasks.
    //  - have high/low priority tasks.
	thread physics_thread;
	thread anim_thread;
	thread misc_thread;

	ThreadSafeQueue<AsyncTask> misc_tasks;
	ThreadSafeQueue<AsyncTask> physics_tasks;
	ThreadSafeQueue<AsyncTask> anim_tasks;
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
