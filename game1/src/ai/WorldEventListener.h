#pragma once 

#include "sausage.h"
#include "AiCommon.h"

using namespace std;

namespace Ai {
	class WorldEventListener {
		vector<WorldEvent> events;
		map<int, vector<WorldEventCallback>> world_callbacks;
	public:
		void Listen(WorldEvent evt) {
			events.push_back(evt);
		};
		void Register(int event_id, WorldEventCallback callback) {
			world_callbacks[event_id].push_back(callback);
		};
		void Update() {
			for (auto& evt : events) {
				if (world_callbacks.find(evt.id) != world_callbacks.end()) {
					for (auto& evt_callback : *world_callbacks) {
						evt_callback(evt);
					}
				}
			}
		};
	};
}