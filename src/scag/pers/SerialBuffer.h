/* $Id$ */

#ifndef SCAG_PERS_SERIAL_BUFFER
#define SCAG_PERS_SERIAL_BUFFER

#include <string>
#include <netinet/in.h>
#include "core/buffers/TmpBuf.hpp"

namespace scag{ namespace pers{

using namespace smsc::core::buffers;

class SerialBufferOutOfBounds{};

typedef TmpBuf<char, 2048> _SerialBuffer;
class SerialBuffer : public _SerialBuffer
{
public:
	SerialBuffer() : _SerialBuffer(2048), size(0) {};
	SerialBuffer(int size) : _SerialBuffer(size), size(0) {};
	void* operator new(size_t sz) { return ::operator new(sz); };
	std::string toString();
	void ReadString(std::string &str);
	void ReadString(std::wstring &str);
	void Read(char* dst, int count);
	void Append(const char* data,int count);
	void Empty();
	uint32_t GetSize();
	uint32_t ReadInt32();
	uint16_t ReadInt16();
	uint8_t ReadInt8();
	void WriteInt32(uint32_t);
	void WriteInt16(uint16_t);
	void WriteInt8(uint8_t);
	void WriteString(const char *str);
	void WriteString(const wchar_t *str);

protected:
	uint32_t size;
};

class Serializable
{
public:
	virtual void Serialize(SerialBuffer &sb) = 0;
	virtual void Deserialize(SerialBuffer &sb) = 0;
};

}}

#endif // SCAG_PERS_SERIAL_BUFFER
