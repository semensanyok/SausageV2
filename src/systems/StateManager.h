#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Settings.h"
#include "BufferStorage.h"

using namespace std;

class StateManager {
	map<unsigned long, pair<MeshData*, mat4>> physics_update;

public:
	float delta_time = 0;
	float last_ticks = 0;
	uint32_t milliseconds_since_start = 0;
	double seconds_since_start = 0;

	pair<MeshData*, mat4>& GetPhysicsUpdate(MeshData* mesh) {
		return physics_update[mesh->id];
	}
	void BufferUpdates() {
		_BufferTransformUpdate();
		{
			shared_lock<shared_mutex> end_render_frame_lock(Events::end_render_frame_mtx);
			Events::end_render_frame_event.wait(end_render_frame_lock);
		}
	}
	void BufferBoneTransformUpdate(BufferStorage* buffer, map<unsigned int, mat4>& final_transforms) {
		buffer->BufferBoneTransform(final_transforms);
	}
	void UpdateDeltaTimeTimings() {
		float this_ticks = SDL_GetTicks();
		delta_time = this_ticks - last_ticks;
		last_ticks = this_ticks;
		milliseconds_since_start = SDL_GetTicks();
		seconds_since_start = (double)milliseconds_since_start / 1000;
	}
	void Reset() {
		physics_update.clear();
		milliseconds_since_start = 0;
		seconds_since_start = 0;
	}
	StateManager() {};
	~StateManager() {};
private:
	void _BufferTransformUpdate() {
		for (auto& mesh_update : physics_update) {
			auto mesh = mesh_update.second.first;
			mesh->transform = mesh_update.second.second;
			mesh->buffer->BufferTransform(mesh);
		}
		physics_update.clear();
	}
};