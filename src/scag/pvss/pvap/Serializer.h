#ifndef __SCAG_PVSS_PVAP_SERIALIZER_H__
#define __SCAG_PVSS_PVAP_SERIALIZER_H__

#include "util/int.h"
#include <memory.h>
#include <arpa/inet.h>
#include <string>
#include <wchar.h>
#include "Exceptions.h"

namespace scag {
namespace pvss {
namespace pvap {

class PVAPBC;

class Serializer
{
public:
    enum{tagTypeSize=2};
    enum{lengthTypeSize=2};
    enum{endOfMessage_tag=0xffff};

public:
    Serializer( size_t size = 256 ) :
    owned(true), buf(0), pos(0), dataSize(0), bufSize(size) {
        if (bufSize) buf = new char[bufSize];
    }
  
    void clear()
    {
        pos = 0;
        dataSize = 0;
        if (owned) delete[] buf;
        buf = 0;
        bufSize = 0;
        owned = true;
    }

    PVAPBC& getPVAPBC();

    void writeTag(uint16_t tag)
    {
        resize(2);
        uint16_t netvalue=htons(tag);
        memcpy(buf+pos,&netvalue,2);
        pos+=2;
    }
    void writeLength(uint16_t length)
    {
        resize(2+length); // optimization (as the next action is writing contents)
        uint16_t netvalue=htons(length);
        memcpy(buf+pos,&netvalue,2);
        pos+=2;
    }
    void writeByte(uint8_t value)
    {
        resize(1);
        memcpy(buf+pos,&value,1);
        pos+=1;
    }
    void writeInt16(uint16_t value)
    {
        resize(2);
        uint16_t netvalue=htons(value);
        memcpy(buf+pos,&netvalue,2);
        pos+=2;
    }
    void writeInt32(uint32_t value)
    {
        resize(4);
        uint32_t netvalue=htonl(value);
        memcpy(buf+pos,&netvalue,4);
        pos+=4;
    }
    void writeStr(const std::string& value)
    {
        resize(value.length()*2);
        //memcpy(buf+pos,value.c_str(),value.length());
        for(size_t i=0;i<value.length();i++)
        {
            writeInt16(btowc((unsigned char)value[i]));
        }
        //pos+=value.length();
    }
    void writeBool(bool value)
    {
        resize(1);
        uint8_t data=value?1:0;
        memcpy(buf+pos,&data,1);
        pos+=1;
    }
    void writeByteLV(uint8_t value)
    {
        writeLength(1);
        writeByte(value);
    }
	
    void writeInt16LV(uint16_t value)
    {
        writeLength(2);
        writeInt16(value);
    }
	
    void writeInt32LV(uint32_t value)
    {
        writeLength(4);
        writeInt32(value);
    }
	
    void writeBoolLV(bool value)
    {
        writeLength(1);
        writeBool(value);
    }
  
    void writeStrLV(const std::string& value)
    {
        writeLength(unsigned(value.length())*2);
        writeStr(value);
    }
	
    bool hasMoreData()
    {
        return pos<dataSize;
    }
  
    uint16_t readLength()
    {
        if(pos+2>dataSize)
        {
            throw ReadBeyondEof();
        }
        uint16_t rv;
        memcpy(&rv,buf+pos,2);
        pos+=2;
        return htons(rv);
    }
    uint16_t readTag()
    {
        if (pos == dataSize) {
            return endOfMessage_tag;
        }
        if(pos+2>dataSize)
        {
            throw ReadBeyondEof();
        }
        uint16_t rv;
        memcpy(&rv,buf+pos,2);
        pos+=2;
        return htons(rv);
    }
    uint8_t readByte()
    {
        if(pos+1>dataSize)
        {
            throw ReadBeyondEof();
        }
        uint8_t rv;
        memcpy(&rv,buf+pos,1);
        pos++;
        return rv;
    }
    uint16_t readInt16()
    {
        if(pos+2>dataSize)
        {
            throw ReadBeyondEof();
        }
        uint16_t rv;
        memcpy(&rv,buf+pos,2);
        pos+=2;
        return htons(rv);
    }
    uint32_t readInt32()
    {
        if(pos+4>dataSize)
        {
            throw ReadBeyondEof();
        }
        uint32_t rv;
        memcpy(&rv,buf+pos,4);
        pos+=4;
        return htonl(rv);
    }
    bool readBool()
    {
        if(pos+1>dataSize)
        {
            throw ReadBeyondEof();
        }
        uint8_t rv;
        memcpy(&rv,buf+pos,1);
        pos+=1;
        return rv==1;
    }

    std::string readStrLV()
    {
        uint16_t length=readLength();
        if(pos+length>dataSize)
        {
            throw ReadBeyondEof();
        }
        std::string rv;
        for(int i=0;i<length/2;i++)
        {
            rv+=wctob(readInt16());
        }
        //rv.assign(buf+pos,length);
        //pos+=length;
        return rv;
    }
	
    bool readBoolLV()
    {
        uint16_t len=readLength();
        if(len!=1)
        {
            throw InvalidValueLength("bool",len);
        }
        return readBool();
    }
	
    bool readByteLV()
    {
        uint16_t len=readLength();
        if(len!=1)
        {
            throw InvalidValueLength("byte",len);
        }
        return readByte();
    }
	
    bool readInt16LV()
    {
        uint16_t len=readLength();
        if(len!=2)
        {
            throw InvalidValueLength("int16",len);
        }
        return readInt16();
    }
	
    bool readInt32LV()
    {
        uint16_t len=readLength();
        if(len!=4)
        {
            throw InvalidValueLength("int32",len);
        }
        return readInt32();
    }

    void skip(uint32_t bytes)
    {
        if(pos+bytes>dataSize)
        {
            throw ReadBeyondEof();
        }
        pos+=bytes;
    }

    std::string getDump()
    {
        char thebuf[32];
        std::string rv;
        for(size_t i=0;i<pos;i++)
        {
            sprintf(thebuf,"%02x ",(unsigned int)(unsigned char)this->buf[i]);
            rv+=thebuf;
        }
        return rv;
    }

    void rewind()
    {
        dataSize=pos;
        pos=0;
    }

    void setData( const char* data,size_t size, bool doCopy = true )
    {
        pos=0;
        if ( doCopy ) {
            resize(size);
            memcpy(buf,data,size);
        } else {
            // using external buffer
            clear();
            owned = false;
            buf = const_cast<char*>(data);
        }
        dataSize=size;
    }

    /// these two methods are useful after writing to buffer
    const char* getData() const
    {
        return buf;
    }
    size_t getPos() const
    {
        return pos;
    }

    /// this method is useful before reading from socket
    char* prepareBuffer(uint32_t size)
    {
        pos=0;
        if ( owned ) {
            resize(size);
        } else {
            clear();
            owned = true;
            bufSize = size;
            buf = new char[bufSize];
        }
        dataSize=size;
        return buf;
    }

protected:
    bool owned;
    char* buf;
    size_t pos;
    size_t dataSize; // is for reading only: end of data pointer
    size_t bufSize;
    void resize(size_t grow)
    {
        if (!owned) throw std::runtime_error("cannot write to unowned buffer");
        if(pos+grow>bufSize)
        {
            bufSize = bufSize*2+grow;
            char* newbuf = new char[bufSize];
            if (buf) {
                if (pos) memcpy(newbuf,buf,pos);
                delete [] buf;
            }
            buf=newbuf;
        }
    }

};

}
}
}

namespace scag2 {
namespace pvss {
namespace pvap = scag::pvss::pvap;
} // namespace pvss
} // namespace scag2

#endif
