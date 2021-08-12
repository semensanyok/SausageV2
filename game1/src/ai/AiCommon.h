#pragma once

namespace Ai {
	struct WorldEvent {
		int id;
	};

	class CoefficientCallback {
	public:
		int coef_index;
		virtual void call();
	};

	class WorldEventCallback {
	public:
		virtual void call(WorldEvent evt);
	};
}