/* $Id$ */

#ifndef SCAG_PERS_SERIAL_BUFFER
#define SCAG_PERS_SERIAL_BUFFER

#include <string>
#include "core/buffers/TmpBuf.hpp"

namespace scag{ namespace pers{

using namespace std;

using namespace smsc::core::buffers;

typedef TmpBuf<char, 2048> _SerialBuffer;
class SerialBuffer : public _SerialBuffer
{
public:
	SerialBuffer() : _SerialBuffer(2048) {};
	SerialBuffer(int size) : _SerialBuffer(size) {};
	void* operator new(size_t sz) { return ::operator new(sz); };
	string toString()
	{
		string str;
		int i = 0, j = GetPos();
		char buf[10];

		uint8_t b;
		SetPos(0);
		while(i++ < j)
		{
			Read((char*)&b, 1);
			sprintf(buf, "%02x(%c)", (int)b, (b > 32 && b < 128 ? b : '*'));
			str += buf;
		}
		SetPos(j);
		return str;
	}
};

class SerializableException{};

class Serializable
{
public:
	virtual void Serialize(SerialBuffer &sb) = 0;
	virtual void Deserialize(SerialBuffer &sb) throw(SerializableException) = 0;
};

}}

#endif // SCAG_PERS_SERIAL_BUFFER
