#include "scag/util/SerializeBuffer.h"

namespace scag{
namespace util{

using namespace smsc::sms;

SerializeBuffer& SerializeBuffer::operator >> (std::string& str)
{
    uint8_t len;
    this->Read((char*)&len,1);
    char scb[256];

    if (len>255) throw smsc::util::Exception("Attempt to read %d byte in buffer with size %d",(int)len,255);

    this->Read(scb,len);
    scb[len] = 0;

    str = scb;
    return *this;
}

SerializeBuffer& SerializeBuffer::operator << (std::string& str)
{
    uint8_t len = uint8_t(str.size());
    if (len>255) throw smsc::util::Exception("Attempt to write %d byte in buffer with size %d",(int)len,255);
        
    this->Append((char*)&len,1);
    this->Append((char*)str.data(),len);

    return *this;
}


SerializeBuffer& SerializeBuffer::operator<<(const Address& addr)
{
    this->Append((char*)&addr.length,1);
    this->Append((char*)&addr.type,1);
    this->Append((char*)&addr.plan,1);
    this->Append(addr.value,addr.length);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(Address& addr)
{
    this->Read((char*)&addr.length,1);
    this->Read((char*)&addr.type,1);
    this->Read((char*)&addr.plan,1);
    this->Read(addr.value,addr.length);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(const char* str)
{
    uint8_t len=uint8_t(strlen(str));
    this->Append((char*)&len,1);
    this->Append(str,len);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(const uint32_t& val)
{
    uint32_t nval=ntohl(val);
    this->Append((char*)&nval,4);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(uint32_t& val)
{
    this->Read((char*)&val,4);
    val=htonl(val);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(const int32_t& val)
{
    uint32_t nval=ntohl(val);
    this->Append((char*)&nval,4);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(int32_t& val)
{
    this->Read((char*)&val,4);
    val=htonl(val);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(const uint64_t& val)
{
    uint32_t nval=ntohl((uint32_t)(val>>32)&0xFFFFFFFF);
    this->Append((char*)&nval,4);
    nval=ntohl((uint32_t)(val&0xFFFFFFFF));
    this->Append((char*)&nval,4);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(uint64_t& val)
{
    uint32_t nval;
    this->Read((char*)&nval,4);
    val=htonl(nval);
    val<<=32;
    this->Read((char*)&nval,4);
    nval=htonl(nval);
    val|=nval;
    return *this;
}


SerializeBuffer& SerializeBuffer::operator<<(time_t val)
{
    uint32_t tmp=uint32_t(ntohl(uint32_t(val)));
    this->Append((char*)&tmp,4);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(time_t& val)
{
    uint32_t tmp;
    this->Read((char*)&tmp,4);
    val=htonl(tmp);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(uint16_t val)
{
    uint16_t nval=ntohs(val);
    this->Append((char*)&nval,2);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(uint16_t& val)
{
    this->Read((char*)&val,2);
    val=htons(val);
    return *this;
}


SerializeBuffer& SerializeBuffer::operator<<(const uint8_t& val)
{
    this->Append((char*)&val,1);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(uint8_t& val)
{
    this->Read((char*)&val,1);
    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(const float& val) {
  if (sizeof(float) != 4) {  
    throw smsc::util::Exception("Cannot serialize float, when sizeof(float)=%d. Not implemented yet.",sizeof(float));
  }
 
#if BYTE_ORDER == BIG_ENDIAN
  this->Append((char*)&val,4);
  return *this;
#else
  const uint32_t *v = reinterpret_cast<const uint32_t*>(&val);
  *this << *v;
  return *this;
#endif 
}

SerializeBuffer& SerializeBuffer::operator>>(float& val) {
  if (sizeof(float) != 4) {  
    throw smsc::util::Exception("Cannot deserialize float, when sizeof(float)=%d. Not implemented yet.",sizeof(float));
  }

#if BYTE_ORDER == BIG_ENDIAN
  this->Read((char*)&val,4);
  return *this;
#else
  uint32_t v; 
  *this >> v;
  val = *(reinterpret_cast<float*>(&v)); 
  return *this;
#endif
}

}}

