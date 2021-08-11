#pragma once

#include "sausage.h"

using namespace std;
static mutex shader_mutex;

class FileWatcher
{
public:
	void AddCallback(string& path, function<void()>& file_change_callback);
	void RemoveCallback(string& path);
	void Watch();
	FileWatcher();
	~FileWatcher();

private:
	map<string, pair<chrono::system_clock::duration, vector<function<void()>>>> watchers;
};
