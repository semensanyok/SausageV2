#pragma once

#include "sausage.h"
#include "Structures.h"

class StateManager {
	map<unsigned long, pair<MeshData*, mat4>> physics_update;
	map<unsigned long, MeshData*> bone_transform_update;
public:
	float delta_time = 0;
	float last_ticks = 0;
	uint32_t milliseconds_since_start = 0;
	uint32_t seconds_since_start = 0;

	pair<MeshData*, mat4>& GetPhysicsUpdate(MeshData* mesh) {
		return physics_update[mesh->id];
	}
	void AddBoneTransformUpdate(MeshData* mesh) {
		bone_transform_update[mesh->id] = mesh;
	}
	void BufferUpdates() {
		_BufferTransformUpdate();
		_BufferBoneTransformUpdate();
	}
	void UpdateDeltaTimeTimings() {
		float this_ticks = SDL_GetTicks();
		delta_time = this_ticks - last_ticks;
		last_ticks = this_ticks;
		milliseconds_since_start = SDL_GetTicks();
		seconds_since_start = milliseconds_since_start / 1000;
	}
	void Reset() {
		physics_update.clear();
		bone_transform_update.clear();
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
	void _BufferBoneTransformUpdate() {
		for (auto& mesh_update : bone_transform_update) {
			auto mesh = mesh_update.second;
			mesh->buffer->BufferBoneTransform(mesh->armature->bones, mesh->armature->num_bones);
		}
		bone_transform_update.clear();
	}
};