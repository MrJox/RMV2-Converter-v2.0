#ifndef HALF_H
#define HALF_H

class HalfFloat
{
public:

	enum { BITS_MANTISSA = 10 };
	enum { BITS_EXPONENT = 5 };

	enum { MAX_EXPONENT_VALUE = 31 };
	enum { BIAS = MAX_EXPONENT_VALUE/2 };

	enum { MAX_EXPONENT = BIAS };
	enum { MIN_EXPONENT = -BIAS };

	enum { MAX_EXPONENT10 = 9 };
	enum { MIN_EXPONENT10 = -9 };

public:

	/** Default constructor. Unitialized by default.
	 */
	inline HalfFloat() {}

	/** Construction from a single-precision float
	 */
	inline HalfFloat(float other)
	{
		IEEESingle f;
		f.Float = other;

		IEEE.Sign = f.IEEE.Sign;

		if (!f.IEEE.Exp)
		{
			IEEE.Frac = 0;
			IEEE.Exp = 0;
		}
		else if (f.IEEE.Exp == 0xff)
		{
			// NaN or INF
			IEEE.Frac = (f.IEEE.Frac != 0) ? 1 : 0;
			IEEE.Exp = 31;
		}
		else
		{
			// regular number
			int new_exp = f.IEEE.Exp - 127;

			if (new_exp<-24)
			{ // this maps to 0
				IEEE.Frac = 0;
				IEEE.Exp = 0;
			}

			else if (new_exp<-14)
			{
				// this maps to a denorm
				IEEE.Exp = 0;
				unsigned int exp_val = (unsigned int)(-14 - new_exp);  // 2^-exp_val
				switch (exp_val)
				{
				case 0:
					IEEE.Frac = 0;
					break;
				case 1: IEEE.Frac = 512 + (f.IEEE.Frac >> 14); break;
				case 2: IEEE.Frac = 256 + (f.IEEE.Frac >> 15); break;
				case 3: IEEE.Frac = 128 + (f.IEEE.Frac >> 16); break;
				case 4: IEEE.Frac = 64 + (f.IEEE.Frac >> 17); break;
				case 5: IEEE.Frac = 32 + (f.IEEE.Frac >> 18); break;
				case 6: IEEE.Frac = 16 + (f.IEEE.Frac >> 19); break;
				case 7: IEEE.Frac = 8 + (f.IEEE.Frac >> 20); break;
				case 8: IEEE.Frac = 4 + (f.IEEE.Frac >> 21); break;
				case 9: IEEE.Frac = 2 + (f.IEEE.Frac >> 22); break;
				case 10: IEEE.Frac = 1; break;
				}
			}
			else if (new_exp>15)
			{ // map this value to infinity
				IEEE.Frac = 0;
				IEEE.Exp = 31;
			}
			else
			{
				IEEE.Exp = new_exp + 15;
				IEEE.Frac = (f.IEEE.Frac >> 13);
			}
		}
	}

	/** Conversion operator to convert from half to float
	 */
	inline operator float() const
	{
		IEEESingle sng;
		sng.IEEE.Sign = IEEE.Sign;

		if (!IEEE.Exp)
		{
			if (!IEEE.Frac)
			{
				sng.IEEE.Frac = 0;
				sng.IEEE.Exp = 0;
			}
			else
			{
				const float half_denorm = (1.0f / 16384.0f);
				float mantissa = ((float)(IEEE.Frac)) / 1024.0f;
				float sgn = (IEEE.Sign) ? -1.0f : 1.0f;
				sng.Float = sgn*mantissa*half_denorm;
			}
		}
		else if (31 == IEEE.Exp)
		{
			sng.IEEE.Exp = 0xff;
			sng.IEEE.Frac = (IEEE.Frac != 0) ? 1 : 0;
		}
		else
		{
			sng.IEEE.Exp = IEEE.Exp + 112;
			sng.IEEE.Frac = (IEEE.Frac << 13);
		}
		return sng.Float;
	}

	inline uint16_t GetBits() const
	{
		return bits;
	}
	
	inline uint16_t& GetBits()
	{
		return bits;
	}

public:

	union 
	{
		uint16_t bits;			// All bits
		struct 
		{
			uint16_t Frac : 10;	// mantissa
			uint16_t Exp  : 5;		// exponent
			uint16_t Sign : 1;		// sign
		} IEEE;
	};


	union IEEESingle
	{
		float Float;
		struct
		{
			uint32_t Frac : 23;
			uint32_t Exp  : 8;
			uint32_t Sign : 1;
		} IEEE;
	};

	union IEEEDouble
	{
		double Double;
		struct {
			uint64_t Frac : 52;
			uint64_t Exp  : 11;
			uint64_t Sign : 1;
		} IEEE;
	};

	// Enums can not store 64 bit values, so we have to use static constants.
	static const uint64_t IEEEDouble_MaxExpontent = 0x7FF;
	static const uint64_t IEEEDouble_ExponentBias = IEEEDouble_MaxExpontent / 2;
};

#endif //	!HALF_H
