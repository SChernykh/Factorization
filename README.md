# Factorization
Fast factorization of numbers less than 2^64. It does trial divisions up to fourth root of N and then 1 or 2 Pollard-Brent calls to get the remaining factors.

All integer divisions are replaced with multiplications by reciprocals, both for trial division step and Pollard-Brent step.

Some performance data: ~296,000 factorizations per second on a single core of Core i7-4770K for numbers around ~10^15
