#include "Random.h"

Random::Random()
{
	auto seed = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());

	m_Engine.seed(seed);
}

bool Random::Chance(double probability)
{
	std::uniform_real_distribution<double> dist(0.0, 1.0);
	return dist(m_Engine) < probability;
}

bool Random::Bool()
{
	return Range<int>(0, 1) == 1;
}

void Random::Reseed(uint64_t seed)
{
	if (seed == 0)
		seed = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());

	m_Engine.seed(seed);
}
