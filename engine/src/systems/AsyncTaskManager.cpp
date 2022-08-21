#include "AsyncTaskManager.h"

using namespace std;

void AsyncTaskManager::Run() {
	physics_thread = thread([&] {while (!GameSettings::quit) {
		RunPhysicsTasks(GameSettings::quit);
	}});
	misc_thread = thread([&] {while (!GameSettings::quit) {
		RunMiscTasks(GameSettings::quit);
	}});
	anim_thread = thread([&] {while (!GameSettings::quit) {
		RunAnimTasks(GameSettings::quit);
	}});
}
void AsyncTaskManager::SubmitPhysTask(function<void()> task, bool is_persistent) {
	physics_tasks.Push({ task, is_persistent });
}
void AsyncTaskManager::SubmitMiscTask(function<void()> task, bool is_persistent) {
	misc_tasks.Push({ task, is_persistent });
}
void AsyncTaskManager::SubmitAnimTask(function<void()> task, bool is_persistent) {
	anim_tasks.Push({ task, is_persistent });
}
AsyncTaskManager::~AsyncTaskManager() {
	physics_thread.join();
	misc_thread.join();
	anim_thread.join();
}
void AsyncTaskManager::RunPhysicsTasks(bool& quit) {
	lock_guard<mutex> pause_phys_lock(Events::pause_phys_mtx);
	auto tasks = physics_tasks.WaitPopAll(quit);
	while (!tasks.empty()) {
		auto& task = tasks.front();
		task.run();
		if (task.is_persistent) {
			physics_tasks.Push(task);
		}
		tasks.pop();
	}
	{
		shared_lock<shared_mutex> end_render_frame_lock(Events::end_render_frame_mtx);
		Events::end_render_frame_event.wait(end_render_frame_lock);
	}
}
void AsyncTaskManager::RunMiscTasks(bool& quit) {
	auto tasks = misc_tasks.WaitPopAll(quit);
	while (!tasks.empty()) {
		auto& task = tasks.front();
		task.run();
		if (task.is_persistent) {
			misc_tasks.Push(task);
		}
		tasks.pop();
	}
}
void AsyncTaskManager::RunAnimTasks(bool& quit) {
	auto tasks = anim_tasks.PopAll();
	while (!tasks.empty()) {
		auto& task = tasks.front();
#ifdef SAUSAGE_PROFILE_ENABLE
		auto proft1 = chrono::steady_clock::now();
#endif
		task.run();
#ifdef SAUSAGE_PROFILE_ENABLE
		ProfTime::anim_update_ns = chrono::steady_clock::now() - proft1;
#endif
		if (task.is_persistent) {
			anim_tasks.Push(task);
		}
		tasks.pop();
	}
	{
		shared_lock<shared_mutex> end_render_frame_lock(Events::end_render_frame_mtx);
		Events::end_render_frame_event.wait(end_render_frame_lock);
	}
}
