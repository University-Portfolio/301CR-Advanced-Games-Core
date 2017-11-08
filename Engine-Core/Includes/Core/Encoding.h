/**
* Encoding and Decoding template overrides for commonly used types (Further implementations should override these for themselves!)
*/
#pragma once
#include "Common.h"
#include "ByteBuffer.h"
#include <memory>


// Let game override encoding, if they need to
#ifndef STR_MAX_ENCODE_LEN
#define STR_MAX_ENCODE_LEN 128
#endif


template<typename T>
void Encode(ByteBuffer& buffer, const T& data)
{
	static_assert(false, "No encode implementation");
}


template<>
inline void Encode<int8>(ByteBuffer& buffer, const int8& data)
{
	buffer.Push(data);
}

template<>
inline void Encode<int16>(ByteBuffer& buffer, const int16& data)
{
	buffer.Push(data >> 0 * 8);
	buffer.Push(data >> 1 * 8);
}

template<>
inline void Encode<int32>(ByteBuffer& buffer, const int32& data)
{
	buffer.Push(data >> 0 * 8);
	buffer.Push(data >> 1 * 8);
	buffer.Push(data >> 2 * 8);
	buffer.Push(data >> 3 * 8);
}

template<>
inline void Encode<int64>(ByteBuffer& buffer, const int64& data)
{
	buffer.Push(data >> 0 * 8);
	buffer.Push(data >> 1 * 8);
	buffer.Push(data >> 2 * 8);
	buffer.Push(data >> 3 * 8);
	buffer.Push(data >> 4 * 8);
	buffer.Push(data >> 5 * 8);
	buffer.Push(data >> 6 * 8);
	buffer.Push(data >> 7 * 8);
}


template<>
inline void Encode<uint8>(ByteBuffer& buffer, const uint8& data)
{
	buffer.Push(data);
}

template<>
inline void Encode<uint16>(ByteBuffer& buffer, const uint16& data)
{
	buffer.Push(data >> 0 * 8);
	buffer.Push(data >> 1 * 8);
}

template<>
inline void Encode<uint32>(ByteBuffer& buffer, const uint32& data)
{
	buffer.Push(data >> 0 * 8);
	buffer.Push(data >> 1 * 8);
	buffer.Push(data >> 2 * 8);
	buffer.Push(data >> 3 * 8);
}

template<>
inline void Encode<uint64>(ByteBuffer& buffer, const uint64& data)
{
	buffer.Push(data >> 0 * 8);
	buffer.Push(data >> 1 * 8);
	buffer.Push(data >> 2 * 8);
	buffer.Push(data >> 3 * 8);
	buffer.Push(data >> 4 * 8);
	buffer.Push(data >> 5 * 8);
	buffer.Push(data >> 6 * 8);
	buffer.Push(data >> 7 * 8);
}


template<>
inline void Encode<float>(ByteBuffer& buffer, const float& data)
{
	int temp;
	std::memcpy(&temp, &data, sizeof(float));
	Encode(buffer, temp);
}

template<>
inline void Encode<const char*>(ByteBuffer& buffer, const char* const& data)
{
	const char* c = data;
	for (uint32 i = 1; i < STR_MAX_ENCODE_LEN; ++i)
	{
		buffer.Push(*c);
		if (*c == '\0')
			return;
		else
			++c;
	}

	// Reached limit
	buffer.Push('\0');
}

template<>
inline void Encode<std::string>(ByteBuffer& buffer, const string& data)
{
	Encode<const char*>(buffer, data.c_str());
}




template<typename T>
bool Decode(ByteBuffer& buffer, T& out, void* context = nullptr)
{
	static_assert(false, "No decode implementation");
	return false;
}

template<>
inline bool Decode<int8>(ByteBuffer& buffer, int8& out, void* context)
{
	out = 0;

	if (buffer.Size() < sizeof(int8))
		return false;

	out += buffer.Pop();
	return true;
}

template<>
inline bool Decode<int16>(ByteBuffer& buffer, int16& out, void* context)
{
	out = 0;

	if (buffer.Size() < sizeof(int16))
		return false;

	out += buffer.Pop() << 0 * 8;
	out += buffer.Pop() << 1 * 8;
	return true;
}

template<>
inline bool Decode<int32>(ByteBuffer& buffer, int32& out, void* context)
{
	out = 0;

	if (buffer.Size() < sizeof(int32))
		return false;

	out += buffer.Pop() << 0 * 8;
	out += buffer.Pop() << 1 * 8;
	out += buffer.Pop() << 2 * 8;
	out += buffer.Pop() << 3 * 8;
	return true;
}

template<>
inline bool Decode<int64>(ByteBuffer& buffer, int64& out, void* context)
{
	out = 0;

	if (buffer.Size() < sizeof(int64))
		return false;

	out += buffer.Pop() << 0 * 8;
	out += buffer.Pop() << 1 * 8;
	out += buffer.Pop() << 2 * 8;
	out += buffer.Pop() << 3 * 8;
	out += buffer.Pop() << 4 * 8;
	out += buffer.Pop() << 5 * 8;
	out += buffer.Pop() << 6 * 8;
	out += buffer.Pop() << 7 * 8;
	return true;
}


template<>
inline bool Decode<uint8>(ByteBuffer& buffer, uint8& out, void* context)
{
	out = 0;

	if (buffer.Size() < sizeof(uint8))
		return false;

	out += buffer.Pop();
	return true;
}

template<>
inline bool Decode<uint16>(ByteBuffer& buffer, uint16& out, void* context)
{
	out = 0;

	if (buffer.Size() < sizeof(uint16))
		return false;

	out += buffer.Pop() << 0 * 8;
	out += buffer.Pop() << 1 * 8;
	return true;
}

template<>
inline bool Decode<uint32>(ByteBuffer& buffer, uint32& out, void* context)
{
	out = 0;

	if (buffer.Size() < sizeof(uint32))
		return false;

	out += buffer.Pop() << 0 * 8;
	out += buffer.Pop() << 1 * 8;
	out += buffer.Pop() << 2 * 8;
	out += buffer.Pop() << 3 * 8;
	return true;
}

template<>
inline bool Decode<uint64>(ByteBuffer& buffer, uint64& out, void* context)
{
	out = 0;

	if (buffer.Size() < sizeof(uint64))
		return false;

	out += buffer.Pop() << 0 * 8;
	out += buffer.Pop() << 1 * 8;
	out += buffer.Pop() << 2 * 8;
	out += buffer.Pop() << 3 * 8;
	out += buffer.Pop() << 4 * 8;
	out += buffer.Pop() << 5 * 8;
	out += buffer.Pop() << 6 * 8;
	out += buffer.Pop() << 7 * 8;
	return true;
}


template<>
inline bool Decode<float>(ByteBuffer& buffer, float& out, void* context)
{
	if (buffer.Size() < sizeof(float))
		return false;

	uint32 temp;
	Decode(buffer, temp);
	std::memcpy(&out, &temp, sizeof(float));
	return true;
}

template<>
inline bool Decode<std::string>(ByteBuffer& buffer, string& out, void* context)
{
	while (true)
	{
		// Emptied before a '\0'
		if (buffer.Size() == 0)
			return false;

		char c = buffer.Pop();

		if (c == '\0')
			return true;

		out += c;
	}

	return true;
}