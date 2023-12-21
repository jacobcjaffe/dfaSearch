#pragma once
#include<vector>

class BitVector 
{
private:
	int numOnes = 0;
	int strLength = 0;
	bool overflow = false;
	std::vector<bool> bits;
public:
	BitVector(int length) {
		bits.resize(length, 0);
		strLength = length;
	}

	// method to add 1 in the bool vector, acts like normal addition, but keeps track
	// of the number of zeros
	void operator++ () {
		if (overflow) {
			std::cout << "overflow on BitVector" << std::endl;
			return;
		}
		for (int i = 0; i < strLength; i++) {
			if (bits[i] == 0) {
				bits[i] = 1;
				numOnes++;
				return;
			}
			else { // bits[i] == 1
				if (i == strLength - 1) {
					overflow = true;
					return;
				}
				else {
					bits[i] = 0;
					numOnes--;
				}
			}
		}
		return;
	}

	/// <summary>
	/// Get a reference to the underlying vector.
	/// </summary>
	/// <returns></returns>
	std::vector<bool>& GetVector() {
		return bits;
	}

	/// <summary>
	/// returns whether the vector has experienced overflow
	/// </summary>
	/// <returns></returns>
	bool IsOverflow() {
		return overflow;
	}

	/// <summary>
	/// returns whether more than half of the vector's values are ones or not
	/// </summary>
	/// <returns></returns>
	bool HalfOrMoreOnes() {
		if (numOnes > ((double)strLength / 2)) {
			return true;
		}
		return false;
	}

	/// <summary>
	/// resets the vector, its overflow flag, and its number of ones
	/// </summary>
	void Reset() {
		for (int i = 0; i < strLength; i++) {
			bits[i] = 0;
		}
		overflow = false;
		numOnes = 0;
		return;
	}

	~BitVector() { }
};
