#pragma once

#include "PrimeTables.h"

struct PrimeInverse
{
	number p;
	number p4;
	number inverse;
	number max_div_value;
};

extern PrimeInverse g_SmallPrimes[6541];

void Factorize(number N, std::vector<std::pair<number, number>>& factorization, number trialStart = 0);
number GetSum(const std::vector<std::pair<number, number>>& factorization);
void NormalizeFactorization(std::vector<std::pair<number, number>>& factorization);

template<typename T, typename U>
NOINLINE void FactorsToString(std::vector<std::pair<T, U>>& factorization, std::string& result)
{
	char buf[32];
	result.clear();
	for (number i = 0; i < factorization.size(); ++i)
	{
		if (i)
		{
			result += '*';
		}
		_ui64toa_s(factorization[i].first, buf, sizeof(buf), 10);
		result += buf;
		if (factorization[i].second > 1)
		{
			result += '^';
			_ui64toa_s(factorization[i].second, buf, sizeof(buf), 10);
			result += buf;
		}
	}
}

template<typename T, typename U>
FORCEINLINE std::string FactorsToString(std::vector<std::pair<T, U>>& factorization)
{
	std::string result;
	FactorsToString(factorization, result);
	return static_cast<std::string&&>(result);
}
