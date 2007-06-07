/* $Id$ */

#ifndef SCAG_PERS_SERIAL_BUFFER
#define SCAG_PERS_SERIAL_BUFFER

#include <string>
#include <netinet/in.h>
#include "DataBlock.h"

//namespace scag{ namespace pers{

//using namespace smsc::core::buffers;

class SerialBufferOutOfBounds{};

class SerialBuffer : public DataBlock
{
public:
    SerialBuffer(){}
    SerialBuffer(int size): DataBlock(size){}

    std::string toString();
    void ReadString(std::string &str);
    void ReadString(std::wstring &str);
    void Read(char* dst, int count);
    void Append(const char* data,int count);
    void Empty();
    uint32_t ReadInt32();
    uint16_t ReadInt16();
    uint8_t ReadInt8();
    void WriteInt32(uint32_t);
    void WriteInt16(uint16_t);
    void WriteInt8(uint8_t);
    void WriteString(const char *str);
    void WriteString(const wchar_t *str);

    uint32_t GetSize() const {return length();}
    size_t GetPos() const {return getPos();};
    const char* GetCurPtr() const {return c_curPtr();};
    void SetPos(size_t _pos) {setPos(_pos);};

protected:

};

class Serializable
{
public:
    virtual void Serialize(SerialBuffer &sb) = 0;
    virtual void Deserialize(SerialBuffer &sb) = 0;
};

//}}

#endif
