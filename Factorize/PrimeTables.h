#pragma once

extern std::vector<byte> MainPrimeTable;
extern byte bitOffset[PrimeTableParameters::Modulo];

void PrimeTablesInit();
number CalculatePrimes(number aLowerBound, number anUpperBound, std::vector<byte>& anOutPrimes);
bool IsPrime(number n);

FORCEINLINE number GCD(number a, number b)
{
	if (a == 0) return b;
	if (b == 0) return a;

	unsigned long shift;
	_BitScanForward64(&shift, a | b);

	unsigned long index_a;
	_BitScanForward64(&index_a, a);
	a >>= index_a;

	do
	{
		unsigned long index_b;
		_BitScanForward64(&index_b, b);
		b >>= index_b;

		const number a1 = a;
		const number b1 = b;
		a = (a1 > b1) ? b1 : a1;
		b = (a1 > b1) ? a1 : b1;

		b -= a;
	} while (b);

	return (a << shift);
}

class PrimeIterator
{
public:
	explicit PrimeIterator(number aStartPrime)
		: mySieveChunk(0xfafd7bbef7ffffffULL & ~number(3))
		, mySieveData((const number*)(MainPrimeTable.data()))
		, myPossiblePrimesForModuloPtr(NumbersCoprimeToModulo)
		, myModuloIndex(0)
		, myBitIndexShift(0)
		, myCurrentPrime(1)
	{
		while (myCurrentPrime < aStartPrime)
			operator++();
	}

	FORCEINLINE number Get() const { return myCurrentPrime; }

	FORCEINLINE PrimeIterator& operator++()
	{
		if (myCurrentPrime < 11)
		{
			myCurrentPrime = (0xB0705320UL >> (myCurrentPrime * 4)) & 15;
			return *this;
		}

		while (!mySieveChunk)
		{
			mySieveChunk = *(++mySieveData);

			const number NewValues = (PrimeTableParameters::Modulo / 2) | (number(PrimeTableParameters::Modulo / 2) << 16) | (number(PrimeTableParameters::Modulo) << 32) |
				(16 << 8) | (32 << 24) | (number(0) << 40);

			myModuloIndex += ((NewValues >> myBitIndexShift) & 255) * 2;
			myBitIndexShift = (NewValues >> (myBitIndexShift + 8)) & 255;

			myPossiblePrimesForModuloPtr = NumbersCoprimeToModulo + myBitIndexShift;
		}

		unsigned long bitIndex;
		_BitScanForward64(&bitIndex, mySieveChunk);
		mySieveChunk &= (mySieveChunk - 1);

		myCurrentPrime = myModuloIndex + myPossiblePrimesForModuloPtr[bitIndex];

		return *this;
	}

private:
	number mySieveChunk;
	const number* mySieveData;
	const unsigned int* myPossiblePrimesForModuloPtr;
	number myModuloIndex;
	number myBitIndexShift;
	number myCurrentPrime;
};
