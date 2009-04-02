/* $Id$ */

#ifndef _SCAG_UTIL_STORAGE_SERIALBUFFER_H
#define _SCAG_UTIL_STORAGE_SERIALBUFFER_H

#include <string>
#include <netinet/in.h>

#include "DataBlock.h"
#include "GlossaryBase.h"

namespace scag { 
namespace util {
namespace storage {

//using namespace smsc::core::buffers;

class SerialBufferOutOfBounds : public std::exception {
public:
    virtual const char* what() const throw () { return "serial buffer out of bounds"; }
};

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
    void SetPos(size_t _pos) {setPos(int(_pos));};

protected:

};


class Serializable
{
public:
    virtual void Serialize(SerialBuffer &sb, bool toFSDB = false, GlossaryBase* glossary = NULL) const = 0;
    virtual void Deserialize(SerialBuffer &sb, bool fromFSDB = false, GlossaryBase* glossary = NULL) = 0;
};

} // namespace storage
} // namespace util
} // namespace scag

namespace scag2 {
namespace util {
namespace storage = scag::util::storage;
}
}

#endif /* ! _SCAG_UTIL_STORAGE_SERIALBUFFER_H */
