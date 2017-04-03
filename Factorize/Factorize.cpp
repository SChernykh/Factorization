#include "stdafx.h"
#include "Factorize.h"
#include "sprp64.h"
#include <algorithm>

PrimeInverse g_SmallPrimes[6541];

struct XORShiftRandomGenerator
{
	FORCEINLINE XORShiftRandomGenerator()
	{
		x = __rdtsc();
	}

	FORCEINLINE number rand64()
	{
		x ^= x >> 12;
		x ^= x << 25;
		x ^= x >> 27;
		return x * number(2685821657736338717);
	}

	number x;
};

NOINLINE number Brent(const number n)
{
	enum { m = 200 };
	const number npi = modular_inverse64(n);

	DWORD highestBitN;
	_BitScanReverse64(&highestBitN, n);
	const number max_a = (number(1) << highestBitN) - 1;

	XORShiftRandomGenerator rnd;
	number g;

	do 
	{
		number a, y, r = 1, q = 1;
		g = 1;

		do
		{
			a = rnd.rand64() & max_a;
		} while (a == 0 || a == n - 2);

		do
		{
			y = rnd.rand64() & max_a;
		} while (y == 0 || y == n - 2);

		do
		{
			const number x = y;
			for (number i = 0; i < r; ++i)
			{
				y = mont_prod_add64(y, y, a, n, npi);
			}

			number k = 0;
			while ((k < r) && (g == 1))
			{
				const number limit1 = r - k;
				const number limit = (limit1 < m) ? limit1 : m;
				for (number i = 0; i < limit; ++i)
				{
					y = mont_prod_add64(y, y, a, n, npi);
					const number b1 = x - y;
					const number b2 = y - x;
					q = mont_prod64(q, (x > y) ? b1 : b2, n, npi);
				}
				g = GCD(q, n);
				k += m;
			}
			r <<= 1;
		} while (g == 1);
	} while (g == n);

	return g;
}

NOINLINE void Factorize(number N, std::vector<std::pair<number, number>>& factorization, number trialStart)
{
	factorization.clear();
	if (N <= 1)
	{
		return;
	}

	DWORD index;
	_BitScanForward64(&index, N);
	if (index > 0)
	{
		N >>= index;
		factorization.emplace_back(std::pair<number, number>(2, index));

		if (N == 1)
		{
			return;
		}
	}

	for (number i = trialStart; i < 80; ++i)
	{
		const PrimeInverse& cur = g_SmallPrimes[i];
		if (cur.p4 > N)
		{
			if (cur.p * cur.p > N)
			{
				factorization.emplace_back(std::pair<number, number>(N, 1));
				return;
			}
			goto TrialDivisionEnd;
		}

		number q = N * cur.inverse;
		if (q <= cur.max_div_value)
		{
			N = q;

			number k = 1;
			for (;;)
			{
				q = N * cur.inverse;
				if (q > cur.max_div_value)
				{
					break;
				}
				N = q;
				++k;
			}

			factorization.emplace_back(std::pair<number, number>(cur.p, k));

			if (N == 1)
			{
				return;
			}
		}
	}

	if (IsPrime(N))
	{
		factorization.emplace_back(std::pair<number, number>(N, 1));
		return;
	}

	for (number i = (trialStart > 80) ? trialStart : 80; i < ARRAYSIZE(g_SmallPrimes); ++i)
	{
		const PrimeInverse& cur = g_SmallPrimes[i];
		if (cur.p4 > N)
		{
			if (cur.p * cur.p > N)
			{
				factorization.emplace_back(std::pair<number, number>(N, 1));
				return;
			}
			goto TrialDivisionEnd2;
		}

		number q = N * cur.inverse;
		if (q <= cur.max_div_value)
		{
			N = q;

			number k = 1;
			for (;;)
			{
				q = N * cur.inverse;
				if (q > cur.max_div_value)
				{
					break;
				}
				N = q;
				++k;
			}

			factorization.emplace_back(std::pair<number, number>(cur.p, k));

			if (N == 1)
			{
				return;
			}

			if (IsPrime(N))
			{
				factorization.emplace_back(std::pair<number, number>(N, 1));
				return;
			}
		}
	}
	goto TrialDivisionEnd2;

	TrialDivisionEnd:

	if (IsPrime(N))
	{
		factorization.emplace_back(std::pair<number, number>(N, 1));
		return;
	}

	TrialDivisionEnd2:

	// N is not prime and has at most 3 factors at this point
	// All factors > N^1/4 here
	number factor1 = Brent(N);
	if (!IsPrime(factor1))
	{
		factor1 = N / factor1;
	}

	number k = 0;
	while ((N % factor1) == 0)
	{
		N /= factor1;
		++k;
	}
	if (N == 1)
	{
		factorization.emplace_back(std::pair<number, number>(factor1, k));
		return;
	}

	if (IsPrime(N))
	{
		if (factor1 < N)
		{
			factorization.emplace_back(std::pair<number, number>(factor1, k));
			factorization.emplace_back(std::pair<number, number>(N, 1));
		}
		else
		{
			factorization.emplace_back(std::pair<number, number>(N, 1));
			factorization.emplace_back(std::pair<number, number>(factor1, k));
		}
	}
	else
	{
		number factor2 = Brent(N);
		N /= factor2;

		if (factor1 > factor2)
		{
			const number t = factor1;
			factor1 = factor2;
			factor2 = t;
		}

		if (N == factor1)
		{
			factorization.emplace_back(std::pair<number, number>(factor1, k + 1));
			factorization.emplace_back(std::pair<number, number>(factor2, 1));
			return;
		}
		else if (N == factor2)
		{
			factorization.emplace_back(std::pair<number, number>(factor1, k));
			factorization.emplace_back(std::pair<number, number>(factor2, 2));
			return;
		}

		if (N < factor1)
		{
			factorization.emplace_back(std::pair<number, number>(N, 1));
			factorization.emplace_back(std::pair<number, number>(factor1, k));
			factorization.emplace_back(std::pair<number, number>(factor2, k));
		}
		else if (N < factor2)
		{
			factorization.emplace_back(std::pair<number, number>(factor1, k));
			factorization.emplace_back(std::pair<number, number>(N, 1));
			factorization.emplace_back(std::pair<number, number>(factor2, k));
		}
		else
		{
			factorization.emplace_back(std::pair<number, number>(factor1, k));
			factorization.emplace_back(std::pair<number, number>(factor2, k));
			factorization.emplace_back(std::pair<number, number>(N, 1));
		}
	}
}

NOINLINE number GetSum(const std::vector<std::pair<number, number>>& factorization)
{
	number result = 1;
	for (const std::pair<number, number>& f : factorization)
	{
		number cur_sum = result;
		for (number k = 0; k < f.second; ++k)
		{
			cur_sum = cur_sum * f.first + result;
		}
		result = cur_sum;
	}
	return result;
}

NOINLINE void NormalizeFactorization(std::vector<std::pair<number, number>>& factorization)
{
	std::sort(factorization.begin(), factorization.end());
	auto j = factorization.begin();
	for (auto i = j + 1; i < factorization.end(); ++i)
	{
		if (i->first == j->first)
		{
			j->second += i->second;
		}
		else
		{
			++j;
			*j = *i;
		}
	}
	factorization.resize(size_t(j + 1 - factorization.begin()));
}

int main()
{
	PrimeTablesInit();

	std::vector<std::pair<number, number>> f;
	f.reserve(16);

	LARGE_INTEGER freq, t1, t2;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&t1);
	for (number i = 1000000000000000; i < 1000000000000000 + 2000000; i += 2)
	{
		Factorize(i, f);
	}
	QueryPerformanceCounter(&t2);

	const double dt = static_cast<double>(t2.QuadPart - t1.QuadPart) / freq.QuadPart;
	printf("%.6f seconds\n%.0f factorizations/second\n", dt, 1e6 / dt);
	getchar();
	return 0;
}
