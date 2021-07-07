#pragma once

#include "sausage.h"
#include "utils/ThreadSafeQueue.h"
#include "Settings.h"

using namespace std;

class AsyncTaskManager {
	int MAX_THREADS = std::thread::hardware_concurrency();
	thread physics_thread;
	thread misc_thread;

	ThreadSafeQueue<pair<function<void()>, bool>> misc_tasks;
	ThreadSafeQueue<pair<function<void()>, bool>> physics_tasks;
public:

	AsyncTaskManager() {};
	void Run() {
		//if (MAX_THREADS >= 3) {
			physics_thread = thread([&] {while (!GameSettings::quit) {
				//this_thread::sleep_for(std::chrono::milliseconds(300));
				RunPhysicsTasks(GameSettings::quit);
			}});
			misc_thread = thread([&] {while (!GameSettings::quit) {
				//this_thread::sleep_for(std::chrono::milliseconds(300));
				RunMiscTasks(GameSettings::quit);
			}});
		//}
		//else {
		//	misc_thread = thread([&] {while (!quit) {
		//		//this_thread::sleep_for(std::chrono::milliseconds(300));
		//		RunMiscTasks();
		//		RunPhysicsTasks();
		//	}});
		//}
	}
	void SubmitPhysTask(function<void()> task, bool is_persistent) {
		physics_tasks.Push({ task, is_persistent });
	}
	void SubmitMiscTask(function<void()> task, bool is_persistent) {
		misc_tasks.Push({ task, is_persistent });
	}
	~AsyncTaskManager() {
		physics_thread.join();
		misc_thread.join();
	}
private:
	void RunPhysicsTasks(bool& quit) {
		auto tasks = physics_tasks.WaitPopAll(quit);
		while (!tasks.empty()) {
			auto& task = tasks.front();
			task.first();
			if (task.second) {
				physics_tasks.Push(task);
			}
			tasks.pop();
		}
		{
			unique_lock<mutex> end_render_frame_lock(Events::end_render_frame_mtx);
			Events::end_render_frame_event.wait(end_render_frame_lock);
		}
	}
	void RunMiscTasks(bool& quit) {
		auto tasks = misc_tasks.WaitPopAll(quit);
		while (!tasks.empty()) {
			auto& task = tasks.front();
			task.first();
			if (task.second) {
				misc_tasks.Push(task);
			}
			tasks.pop();
		}
	}
};