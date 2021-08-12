#pragma once

#include "sausage.h"

using namespace std;

namespace Ai {

	struct CommonwealthInclination {
		float production;
		float trade;
		float farming;
		float culture;
		float science;
		float external_agression;

		float authocrative;
		float republic;

		float totalitarian;
		float democratic;
		float oligarchy;
	};

	struct EconomyState {
		// gross domestic product.
		float gdp;

		float exports;
		float imports;
	};

	struct CommonwealthState {
		EconomyState economy_state;

		float population;
		float territory_size;

		float city_population;
		float city_size;
		float rural_population;

		float literacy;
		float happiness;
		float hungriness;

		float num_workers;
		float num_farmers;
		float num_warriors;

		float unemployment_rate;
		float corruption_rate;

		//
		float wars_won;
		float wars_lost;
		float contribution_pay;
		float contribution_gain;
		float casualities;
	};

	struct Commoddities {
		float crops;
		float meat;
		float fresh_water;
		float livestock;

		float vegetables;
		float fruits;
		float spices;

		float iron;
		float bronze;
		float silver;
		float gold;

		float slaves;
	};

	struct CommonwealthAssets {
		Commoddities commoddities;
		float currency;
	};

	class CommonwealthBehaviour {
		vector<CoefficientCallback> coefficients_callbacks;

		void RunCallbacks() {

		};

		void CorrectCoefficients() {

		};
	};
}