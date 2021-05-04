// for fun xor string encryption
#pragma once

#ifndef _XOR_H_
#define _XOR_H_

#pragma warning (disable: 4996)

#include <string>
#include <array>
#include <cstdarg>

#define BEGIN_NAMESPACE( x ) namespace x {
#define END_NAMESPACE }

BEGIN_NAMESPACE(XorCompileTime)
constexpr auto time = __TIME__;
constexpr auto seed = static_cast<int>(time[7]) + static_cast<int>(time[6]) * 10 + static_cast<int>(time[4]) * 60 + static_cast<int>(time[3]) * 600 + static_cast<int>(time[1]) * 3600 + static_cast<int>(time[0]) * 36000;

// 1988, Stephen Park and Keith Miller
// "Random Number Generators: Good Ones Are Hard To Find", considered as "minimal standard"
// Park-Miller 31 bit pseudo-random number generator, implemented with G. Carta's optimisation:
// with 32-bit math and without division

template <int N>
struct RandomGenerator
{
private:
	static constexpr unsigned a = 16807; // 7^5
	static constexpr unsigned m = 2147483647; // 2^31 - 1

	static constexpr unsigned s = RandomGenerator<N - 1>::value;
	static constexpr unsigned lo = a * (s & 0xFFFF); // Multiply lower 16 bits by 16807
	static constexpr unsigned hi = a * (s >> 16); // Multiply higher 16 bits by 16807
	static constexpr unsigned lo2 = lo + ((hi & 0x7FFF) << 16); // Combine lower 15 bits of hi with lo's upper bits
	static constexpr unsigned hi2 = hi >> 15; // Discard lower 15 bits of hi
	static constexpr unsigned lo3 = lo2 + hi;

public:
	static constexpr unsigned max = m;
	static constexpr unsigned value = lo3 > m ? lo3 - m : lo3;
};

template <>
struct RandomGenerator<0>
{
	static constexpr unsigned value = seed;
};

template <int N, int M>
struct RandomInt
{
	static constexpr auto value = RandomGenerator<N + 1>::value % M;
};

template <int N>
struct RandomChar
{
	static const char value = static_cast<char>(1 + RandomInt<N, 0x7F - 1>::value);
};

template <size_t N, int K>
struct XorString
{
private:
	const char _key;
	std::array<char, N + 1> _encrypted;

	constexpr char enc(char c) const
	{
		return c ^ _key;
	}

	char dec(char c) const
	{
		return c ^ _key;
	}

public:
	template <size_t... Is>
	constexpr __forceinline XorString(const char* str, std::index_sequence<Is...>) : _key(RandomChar<K>::value), _encrypted{ enc(str[Is])... }
	{
	}

	__forceinline decltype(auto) decrypt(void)
	{
		for (size_t i = 0; i < N; ++i)
		{
			_encrypted[i] = dec(_encrypted[i]);
		}
		_encrypted[N] = '\0';
		return _encrypted.data();
	}
};

//-- Note: XorStr will __NOT__ work directly with functions like printf.
//         To work with them you need a wrapper function that takes a const char*
//         as parameter and passes it to printf and alike.
//
//         The Microsoft Compiler/Linker is not working correctly with variadic
//         templates!
//
//         Use the functions below or use std::cout (and similar)!

static auto w_printf = [](const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf_s(fmt, args);
	va_end(args);
};

static auto w_printf_s = [](const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf_s(fmt, args);
	va_end(args);
};

static auto w_sprintf = [](char* buf, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
};

static auto w_sprintf_s = [](char* buf, size_t buf_size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, buf_size, fmt, args);
	va_end(args);
};
static bool w_strcmp(const char* str1, const char* str2)
{
	return strcmp(str1, str2);
};

#ifdef NDEBUG
#define _xor_( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define _xor_( s ) ( s )
#endif

/*#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#endif
#define XorStr( s ) ( s )
#endif*/

#ifdef ENABLE_XOR
#define XorStr _xor_
#else
#define XorStr
#endif

END_NAMESPACE

#endif // !_XOR_H_