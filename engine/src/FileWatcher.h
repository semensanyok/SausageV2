#pragma once

#include "sausage.h"
#include "structures/Structures.h"
#include "structures/Interfaces.h"

using namespace std;
static mutex shader_mutex;

class FileWatcher : public SausageSystem
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
