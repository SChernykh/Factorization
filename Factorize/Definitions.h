#pragma once

typedef unsigned char byte;
typedef unsigned long long int number;

enum PrimeTableParameters : number
{
	Bound = 5000000000,
	ShiftMultiplier = 2,
	ShiftTableSize = 234954223,
	Modulo = 210,
	NumOffsets = 48,
};

static_assert(PrimeTableParameters::Modulo < 256, "Modulo is too large");

const unsigned int NumbersCoprimeToModulo[NumOffsets * 2] = {
	  1,  11,  13,  17,  19,  23,  29,  31,
	 37,  41,  43,  47,  53,  59,  61,  67,
	 71,  73,  79,  83,  89,  97, 101, 103,
	107, 109, 113, 121, 127, 131, 137, 139,
	143, 149, 151, 157, 163, 167, 169, 173,
	179, 181, 187, 191, 193, 197, 199, 209,

	  1 + 210,  11 + 210,  13 + 210,  17 + 210,  19 + 210,  23 + 210,  29 + 210,  31 + 210,
	 37 + 210,  41 + 210,  43 + 210,  47 + 210,  53 + 210,  59 + 210,  61 + 210,  67 + 210,
	 71 + 210,  73 + 210,  79 + 210,  83 + 210,  89 + 210,  97 + 210, 101 + 210, 103 + 210,
	107 + 210, 109 + 210, 113 + 210, 121 + 210, 127 + 210, 131 + 210, 137 + 210, 139 + 210,
	143 + 210, 149 + 210, 151 + 210, 157 + 210, 163 + 210, 167 + 210, 169 + 210, 173 + 210,
	179 + 210, 181 + 210, 187 + 210, 191 + 210, 193 + 210, 197 + 210, 199 + 210, 209 + 210,
};

enum Byte
{
	Bits = 8,
};

extern unsigned __int64(__fastcall *udiv128)(unsigned __int64 numhi, unsigned __int64 numlo, unsigned __int64 den, unsigned __int64* rem);
extern unsigned __int64(__fastcall *mulmod64)(number a, number b, number n);
