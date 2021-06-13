#pragma once

#include "sausage.h"

using namespace std;
mutex shader_mutex;

class FileWatcher
{
public:
	void AddCallback(string& path, function<void()>& file_change_callback);
	void RemoveCallback(string& path);
	void Start(bool& quit);
	void Join();
	FileWatcher();
	~FileWatcher();

private:
	map<string, pair<chrono::system_clock::duration, vector<function<void()>>>> watchers;
	thread t;
};

void FileWatcher::RemoveCallback(string& path)
{
	lock_guard lock_shader(shader_mutex);
	watchers.erase(path);
};

void FileWatcher::AddCallback(string& path, function<void()>& file_change_callback)
{
	lock_guard lock_shader(shader_mutex);
	auto& watcher = watchers[path];
	if (watcher.second.empty()) {
		watcher.first = filesystem::last_write_time(path).time_since_epoch();
	}
	watcher.second.push_back(file_change_callback);
};

void FileWatcher::Join() {
	if (t.joinable()) {
		t.join();
	}
}

void FileWatcher::Start(bool& quit)
{
	if (t.joinable()) {
		return;
	}
	t = thread([&] {
		while (!quit) {
			this_thread::sleep_for(std::chrono::milliseconds(500));
			lock_guard lock_shader(shader_mutex);
			for (auto &watcher : watchers) {
				auto wtime_dur = filesystem::last_write_time(watcher.first);
				auto wtime = wtime_dur.time_since_epoch();
				if (wtime > watcher.second.first) {
					for (auto callback : watcher.second.second) {
						callback();
					}
					watcher.second.first = wtime;
				}
			}
		}});
}

FileWatcher::FileWatcher()
{
}

FileWatcher::~FileWatcher()
{
}