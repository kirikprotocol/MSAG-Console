#ifndef __SMSC_UTIL_SERIALIZE_BUFFER_H__
#define __SMSC_UTIL_SERIALIZE_BUFFER_H__

#include "sms/sms.h"
#include "core/buffers/TmpBuf.hpp"
#include "util/Exception.hpp"

namespace scag{
namespace util{

using namespace smsc::sms;
//using namespace smsc::sms::BufOps;

/*
class SerializeBuffer : public TmpBuf<char,2048>
{
public:
    SerializeBuffer& operator >> (std::string& str)
    {
        uint8_t len;
        this->Read((char*)&len,1);
        char scb[256];

        if (len>255) throw smsc::util::Exception("Attempt to read %d byte in buffer with size %d",(int)len,255);

        this->Read(scb,len);
        scb[len] = 0;

        str = scb;
        return *this;
    };

    SerializeBuffer& operator << (std::string& str)
    {
        uint8_t len = str.size();
        if (len>255) throw smsc::util::Exception("Attempt to write %d byte in buffer with size %d",(int)len,255);

        this->Append((char*)&len,1);
        this->Append((char*)str.data(),len);

        return *this;
    }


    SerializeBuffer() : smsc::sms::BufOps::SmsBuffer(2048) {}
    SerializeBuffer(int size):smsc::sms::BufOps::SmsBuffer(size){}
};
*/


class SerializeBuffer : public smsc::core::buffers::TmpBuf<char,2048>
{
public:
    SerializeBuffer& operator >> (std::string& str);
    SerializeBuffer& operator << (std::string& str);
    SerializeBuffer& operator<<(const Address& addr);
    SerializeBuffer& operator>>(Address& addr);
    SerializeBuffer& operator<<(const char* str);
    SerializeBuffer& operator<<(const uint32_t& val);
    SerializeBuffer& operator>>(uint32_t& val);
    SerializeBuffer& operator<<(const int32_t& val);
    SerializeBuffer& operator>>(int32_t& val);
    SerializeBuffer& operator<<(const uint64_t& val);
    SerializeBuffer& operator>>(uint64_t& val);
    SerializeBuffer& operator<<(time_t val);
    SerializeBuffer& operator>>(time_t& val);
    SerializeBuffer& operator<<(uint16_t val);
    SerializeBuffer& operator>>(uint16_t& val);
    SerializeBuffer& operator<<(const uint8_t& val);
    SerializeBuffer& operator>>(uint8_t& val);
    SerializeBuffer& operator<<(const float& val);
    SerializeBuffer& operator>>(float& val);
    SerializeBuffer() : smsc::core::buffers::TmpBuf<char,2048>(2048) {}
    SerializeBuffer(int size): smsc::core::buffers::TmpBuf<char,2048>(size){}

};





}}
#endif
