// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_INC_RESP_HPP__
#define __SCAG_PVSS_PVAP_PC_INC_RESP_HPP__

#include "util/int.h"
#include <string>
#include "Exceptions.h"
#include "TypeId.h"


namespace scag{
namespace pvss{
namespace pvap{

// class PVAP;

class PC_INC_RESP  
{
public:
    PC_INC_RESP()
    {
        clear();
    }
    void clear()
    {
        statusFlag=false;
        intValueFlag=false;
    }

    std::string toString() const
    {
        std::string rv("PC_INC_RESP:");
        char buf[32];
        sprintf(buf,"seqNum=%d",seqNum);
        rv+=buf;
        if(statusFlag) {
            rv+=";status=";
            sprintf(buf,"%u",(unsigned int)status);
            rv+=buf;
        }
        if(intValueFlag) {
            rv+=";intValue=";
            sprintf(buf,"%u",(unsigned int)intValue);
            rv+=buf;
        }
        return rv;
    }

    /*
    template <class DataStream> uint32_t length()const
    {
        uint32_t rv=0;
        if (statusFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(status);
        }
        if (intValueFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(intValue);
        }
        return rv;
    }
     */

    uint8_t getStatus() const
        throw (FieldIsNullException)
    {
        if (!statusFlag) {
            throw FieldIsNullException("status");
        }
        return status;
    }
    void setStatus(uint8_t value)
    {
        status=value;
        statusFlag=true;
    }
    bool hasStatus()const
    {
        return statusFlag;
    }
    uint32_t getIntValue() const
        throw (FieldIsNullException)
    {
        if (!intValueFlag) {
            throw FieldIsNullException("intValue");
        }
        return intValue;
    }
    void setIntValue(uint32_t value)
    {
        intValue=value;
        intValueFlag=true;
    }
    bool hasIntValue()const
    {
        return intValueFlag;
    }

    template <class Proto, class DataStream>
        void serialize( const Proto& proto, DataStream& ds ) const throw (PvapException)
    {
        checkFields();
        // mandatory fields
        printf( "write pos=%d field=%d\n", ds.getPos(), statusTag );
        ds.writeTag(statusTag);
        ds.writeByteLV(status);
        printf( "write pos=%d field=%d\n", ds.getPos(), intValueTag );
        ds.writeTag(intValueTag);
        ds.writeInt32LV(intValue);
        // optional fields
    }

    template <class Proto, class DataStream> void deserialize(const Proto& proto, DataStream& ds)
        throw (PvapException)
    {
        clear();
        while (true) {
            int pos = int(ds.getPos());
            int tag = ds.readTag();
            printf( "read pos=%d field=%d\n", pos, tag );
            if ( tag == -1 ) break;
            switch(tag) {
            case statusTag: {
                if (statusFlag) {
                    throw DuplicateFieldException("status");
                }
                status=ds.readByteLV();
                statusFlag=true;
                break;
            }
            case intValueTag: {
                if (intValueFlag) {
                    throw DuplicateFieldException("intValue");
                }
                intValue=ds.readInt32LV();
                intValueFlag=true;
                break;
            }
            default:
                throw NotImplementedException("reaction of reading unknown");
            }
        }
        checkFields();
    }

    uint32_t getSeqNum() const
    {
        return seqNum;
    }
 
    void setSeqNum(uint32_t value)
    {
        seqNum=value;
    }

protected:
    void checkFields() const throw (MandatoryFieldMissingException)
    {
        // checking mandatory fields
        if (!statusFlag) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "status", "PC_INC_RESP");
            throw MandatoryFieldMissingException(buf);
        }
        if (!intValueFlag) {
            char buf[256];
            snprintf( buf, sizeof(buf), "field=%s msg=%s", "intValue", "PC_INC_RESP");
            throw MandatoryFieldMissingException(buf);
        }
        // checking optional fields
    }

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    static const int statusTag=1;
    static const int intValueTag=10;

    uint32_t seqNum;

    uint8_t status;
    uint32_t intValue;

    bool statusFlag;
    bool intValueFlag;
};

} // namespace scag
} // namespace pvss
} // namespace pvap
#endif
