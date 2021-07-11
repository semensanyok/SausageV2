#pragma once

#include "sausage.h"
#include "utils/ThreadSafeQueue.h"
#include "Settings.h"

using namespace std;

class AsyncTaskManager {
	int MAX_THREADS = std::thread::hardware_concurrency();
	thread physics_thread;
	thread anim_thread;
	thread misc_thread;

	ThreadSafeQueue<pair<function<void()>, bool>> misc_tasks;
	ThreadSafeQueue<pair<function<void()>, bool>> physics_tasks;
public:

	AsyncTaskManager() {};
	//void Reset() {
	//	misc_tasks.PopAll();
	//	physics_tasks.PopAll();
	//}
	void Run() {
		physics_thread = thread([&] {while (!GameSettings::quit) {
			RunPhysicsTasks(GameSettings::quit);
		}});
		misc_thread = thread([&] {while (!GameSettings::quit) {
			RunMiscTasks(GameSettings::quit);
		}});
		//anim_thread = 
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
		lock_guard<mutex> pause_phys_lock(Events::pause_phys_mtx);
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