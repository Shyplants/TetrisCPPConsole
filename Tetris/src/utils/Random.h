#pragma once

#include <random>
#include <chrono>
#include <type_traits>

class Random
{
public:
	Random();
	~Random() = default;

	// 정수형 랜덤값 반환, 범위: [min..max]
	template <typename T>
	T Range(T min, T max)
	{
		static_assert(std::is_integral_v<T>, "Range() requires an integral type!");
		std::uniform_int_distribution<T> dist(min, max);
		return dist(m_Engine);
	}

	// 실수형 랜덤값 반환, 범위: [min..max)
	template <typename T>
	T RangeReal(T min, T max)
	{
		static_assert(std::is_floating_point_v<T>, "RangeReal() requires a floating-point type");
		std::uniform_real_distribution<T> dist(min, max);
		return dist(m_Engine);
	}

	// 확률 테스트 (0~1 난수)
	bool Chance(double probability);
	
	// 랜덤 불리언 (50% 확률)
	bool Bool();

	// 시드 재설정
	void Reseed(uint64_t seed = 0);

	std::mt19937_64& Engine() noexcept { return m_Engine; }
	operator std::mt19937_64& () noexcept { return m_Engine; }

private:
	std::mt19937_64 m_Engine{};
};