#include "stdafx.h"
#include "PrimeTables.h"
#include "Factorize.h"
#include <algorithm>
#include <process.h>
#include "sprp64.h"

std::vector<byte> MainPrimeTable;
byte bitOffset[PrimeTableParameters::Modulo];
number bitMask[PrimeTableParameters::Modulo];

number CalculatePrimes(number aLowerBound, number anUpperBound, std::vector<byte>& anOutPrimes)
{
	aLowerBound -= (aLowerBound % PrimeTableParameters::Modulo);

	static __declspec(thread) unsigned char* locGetRemainderCode = 0;
	unsigned char* GetRemainderCode = locGetRemainderCode;
	if (!GetRemainderCode)
	{
		GetRemainderCode = locGetRemainderCode = (unsigned char*) VirtualAlloc(0, 16, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		unsigned char code[] =
		{
			0xB8,0,0,0,0, // mov eax, 0
			0xBA,0,0,0,0, // mov edx, 0
			0xF7,0xF1,    // div ecx
			0x8B,0xC2,    // mov eax, edx
			0xC3,         // ret
		};
		memcpy(GetRemainderCode, code, sizeof(code));
	}
	*(unsigned int*)(GetRemainderCode + 1) = static_cast<unsigned int>(aLowerBound);
	*(unsigned int*)(GetRemainderCode + 6) = static_cast<unsigned int>(aLowerBound >> 32);
	unsigned __int64(__fastcall *GetRemainder)(unsigned __int64) = (unsigned __int64(__fastcall*)(unsigned __int64))((void*)(GetRemainderCode));

	// https://en.wikipedia.org/wiki/Prime_gap#Numerical_results
	// Since we operate in the range 1..2^64, a gap = PrimeTableParameters::Modulo * 9 = 1890 is enough
	anUpperBound = ((anUpperBound / PrimeTableParameters::Modulo) + 10) * PrimeTableParameters::Modulo;
	const size_t arraySize = static_cast<size_t>((anUpperBound - aLowerBound + PrimeTableParameters::Modulo) / PrimeTableParameters::Modulo * (PrimeTableParameters::NumOffsets / Byte::Bits));
	anOutPrimes.resize(arraySize);
	byte* primes = anOutPrimes.data();
	memset(primes, -1, arraySize);
	number d = 11;
	number sqr_d = 121;

	const number* sieveData = (const number*)(MainPrimeTable.data()) + 1;
	number moduloIndex = 0;
	number bitIndexShift = 0;
	// Precomputed first 64 bits of PrimesUpToSqrtLimit
	// They're not available when we're only starting to calculate PrimesUpToSqrtLimit table
	// So I just hard-coded them here
	number curSieveChunk = 0xfafd7bbef7ffffffULL & ~number(3);
	const unsigned int* PossiblePrimesForModuloPtr = NumbersCoprimeToModulo;

	const number rangeSize = anUpperBound - aLowerBound;
	const number GetRemainderBound = aLowerBound >> 32;
	while (sqr_d <= anUpperBound)
	{
		number k = sqr_d;
		if (k < aLowerBound)
		{
			if (d * 2 > GetRemainderBound)
				break;
			number k1 = aLowerBound % (d * 2);
			number correction = 0;
			if (k1 > d)
				correction = d * 2;
			k = d + correction - k1;
		}
		else
			k -= aLowerBound;

		for (; k <= rangeSize; k += d * 2)
		{
			const number mask = bitMask[k % PrimeTableParameters::Modulo];
			if (mask)
			{
				number* chunk = reinterpret_cast<number*>(primes + (k / PrimeTableParameters::Modulo) * (PrimeTableParameters::NumOffsets / Byte::Bits));
				*chunk &= mask;
			}
		}

		while (!curSieveChunk)
		{
			curSieveChunk = *(sieveData++);

			const number NextValuesModuloIndex = (PrimeTableParameters::Modulo / 2) | (number(PrimeTableParameters::Modulo / 2) << 16) | (number(PrimeTableParameters::Modulo) << 32);
			const number NextValuesBitIndexShift = 16 | (32 << 16) | (number(0) << 32);

			moduloIndex += ((NextValuesModuloIndex >> bitIndexShift) & 255) * 2;
			bitIndexShift = (NextValuesBitIndexShift >> bitIndexShift) & 255;

			PossiblePrimesForModuloPtr = NumbersCoprimeToModulo + bitIndexShift;
		}

		unsigned long bitIndex;
		_BitScanForward64(&bitIndex, curSieveChunk);
		curSieveChunk &= (curSieveChunk - 1);

		d = moduloIndex + PossiblePrimesForModuloPtr[bitIndex];
		sqr_d = d * d;
	}

	while (sqr_d <= anUpperBound)
	{
		number k = sqr_d;
		if (k < aLowerBound)
		{
			const number k1 = GetRemainder(d * 2);
			number correction = 0;
			if (k1 > d)
				correction = d * 2;
			k = d + correction - k1;
		}
		else
			k -= aLowerBound;

		for (; k <= rangeSize; k += d * 2)
		{
			const number mask = bitMask[k % PrimeTableParameters::Modulo];
			if (mask)
			{
				number* chunk = reinterpret_cast<number*>(primes + (k / PrimeTableParameters::Modulo) * (PrimeTableParameters::NumOffsets / Byte::Bits));
				*chunk &= mask;
			}
		}

		while (!curSieveChunk)
		{
			curSieveChunk = *(sieveData++);

			const number NextValuesModuloIndex = (PrimeTableParameters::Modulo / 2) | (number(PrimeTableParameters::Modulo / 2) << 16) | (number(PrimeTableParameters::Modulo) << 32);
			const number NextValuesBitIndexShift = 16 | (32 << 16) | (number(0) << 32);

			moduloIndex += ((NextValuesModuloIndex >> bitIndexShift) & 255) * 2;
			bitIndexShift = (NextValuesBitIndexShift >> bitIndexShift) & 255;

			PossiblePrimesForModuloPtr = NumbersCoprimeToModulo + bitIndexShift;
		}

		unsigned long bitIndex;
		_BitScanForward64(&bitIndex, curSieveChunk);
		curSieveChunk &= (curSieveChunk - 1);

		d = moduloIndex + PossiblePrimesForModuloPtr[bitIndex];
		sqr_d = d * d;
	}
	return aLowerBound;
}

bool IsPrime(number n)
{
	if (n >= PrimeTableParameters::Bound)
	{
		return efficient_mr64(n);
	}

	if (n <= 63)
	{
		const number mask = 
			(number(1) << 2) |
			(number(1) << 3) |
			(number(1) << 5) |
			(number(1) << 7) |
			(number(1) << 11) |
			(number(1) << 13) |
			(number(1) << 17) |
			(number(1) << 19) |
			(number(1) << 23) |
			(number(1) << 29) |
			(number(1) << 31) |
			(number(1) << 37) |
			(number(1) << 41) |
			(number(1) << 43) |
			(number(1) << 47) |
			(number(1) << 53) |
			(number(1) << 59) |
			(number(1) << 61);

		return (mask & (number(1) << n)) != 0;
	}

	const number bit = bitOffset[n % PrimeTableParameters::Modulo];
	const number k = (n / PrimeTableParameters::Modulo) * PrimeTableParameters::NumOffsets + bit;
	if (bit >= PrimeTableParameters::NumOffsets)
		return false;

	return (MainPrimeTable[k / Byte::Bits] & (1 << (k % Byte::Bits))) != 0;
}

void PrimeTablesInit()
{
	memset(bitOffset, -1, sizeof(bitOffset));
	for (byte b = 0; b < PrimeTableParameters::NumOffsets; ++b)
	{
		bitOffset[NumbersCoprimeToModulo[b]] = b;
		bitMask[NumbersCoprimeToModulo[b]] = ~(1ULL << b);
	}

	CalculatePrimes(0, PrimeTableParameters::Bound, MainPrimeTable);

	number index = 0;
	for (PrimeIterator it(3); index < ARRAYSIZE(g_SmallPrimes); ++it, ++index)
	{
		const number q = it.Get();
		#pragma warning(suppress : 4146)
		const number q_inv = -modular_inverse64(q);
		const number q_max = number(-1) / q;
		PrimeInverse& inv = g_SmallPrimes[index];
		inv.p = q;
		const number p2 = inv.p * inv.p;
		inv.p4 = p2 * p2;
		inv.inverse = q_inv;
		inv.max_div_value = q_max;
	}
}
