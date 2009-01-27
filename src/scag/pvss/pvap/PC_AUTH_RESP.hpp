// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_AUTH_RESP_HPP__
#define __SCAG_PVSS_PVAP_PC_AUTH_RESP_HPP__

#include "util/int.h"
#include <string>
#include "Exceptions.h"


namespace scag{
namespace pvss{
namespace pvap{

class PC_AUTH_RESP  
{
public:
    PC_AUTH_RESP()
    {
        clear();
    }
    void clear()
    {
        statusFlag=false;
        clientTypeFlag=false;
        sidFlag=false;
    }

    std::string toString() const
    {
        std::string rv("PC_AUTH_RESP:");
        char buf[32];
        sprintf(buf,"seqNum=%d",seqNum);
        rv+=buf;
        if(statusFlag) {
            rv+=";status=";
            sprintf(buf,"%u",(unsigned int)status);
            rv+=buf;
        }
        if(clientTypeFlag) {
            rv+=";clientType=";
            sprintf(buf,"%u",(unsigned int)clientType);
            rv+=buf;
        }
        if(sidFlag) {
            rv+=";sid=";
            sprintf(buf,"%u",(unsigned int)sid);
            rv+=buf;
        }
        return rv;
    }

    template <class DataStream> uint32_t length()const
    {
        uint32_t rv=0;
        if (statusFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(status);
        }
        if (clientTypeFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(clientType);
        }
        if (sidFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(sid);
        }
        return rv;
    }

  uint8_t getStatus() const
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
  uint8_t getClientType() const
    {
        if (!clientTypeFlag) {
            throw FieldIsNullException("clientType");
        }
        return clientType;
    }

    void setClientType(uint8_t value)
    {
        clientType=value;
        clientTypeFlag=true;
    }
    bool hasClientType()const
    {
        return clientTypeFlag;
    }
  uint8_t getSid() const
    {
        if (!sidFlag) {
            throw FieldIsNullException("sid");
        }
        return sid;
    }

    void setSid(uint8_t value)
    {
        sid=value;
        sidFlag=true;
    }
    bool hasSid()const
    {
        return sidFlag;
    }

    template <class DataStream> void serialize(DataStream& ds) const
    {
        checkFields();
        // mandatory fields
        ds.writeTag(statusTag);
    ds.writeByteLV(status);
        // optional fields
        if (clientTypeFlag) {
            ds.writeTag(clientTypeTag);
      ds.writeByteLV(clientType);
        }
        if (sidFlag) {
            ds.writeTag(sidTag);
      ds.writeByteLV(sid);
        }
        //ds.writeTag(DataStream::endOfMessage_tag);
    }

    template <class DataStream> void deserialize(DataStream& ds)
    {
        clear();
        bool endOfMessage=false;
        //uint8_t rdVersionMajor=ds.readByte();
        //uint8_t rdVersionMinor=ds.readByte();
        //if(rdVersionMajor!=versionMajor)
        //{
        //  throw IncompatibleVersionException("PC_AUTH_RESP");
        //}
        //seqNum=ds.readInt32();
        while (!endOfMessage) {
            uint32_t tag=ds.readTag();
            switch(tag) {
            case statusTag: {
                if (statusFlag) {
                    throw DuplicateFieldException("status");
                }
          status=ds.readByteLV();
                statusFlag=true;
                break;
            }
            case clientTypeTag: {
                if (clientTypeFlag) {
                    throw DuplicateFieldException("clientType");
                }
          clientType=ds.readByteLV();
                clientTypeFlag=true;
                break;
            }
            case sidTag: {
                if (sidFlag) {
                    throw DuplicateFieldException("sid");
                }
          sid=ds.readByteLV();
                sidFlag=true;
                break;
            }
            case DataStream::endOfMessage_tag:
                endOfMessage=true;
                break;
            default:
                //if(rdVersionMinor==versionMinor)
                //{
                //  throw UnexpectedTag("PC_AUTH_RESP",tag);
                //}
                ds.skip(ds.readLength());
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
            throw MandatoryFieldMissingException("status");
        }
        // checking optional fields
        if (!clientTypeFlag
            && (status==0)
            ) {
            throw MandatoryFieldMissingException("clientType");
        }
        if (!sidFlag
            && (status==0)
            ) {
            throw MandatoryFieldMissingException("sid");
        }
    }

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    static const uint16_t statusTag=1;
    static const uint16_t clientTypeTag=19;
    static const uint16_t sidTag=20;

    uint32_t seqNum;

    uint8_t status;
    uint8_t clientType;
    uint8_t sid;

    bool statusFlag;
    bool clientTypeFlag;
    bool sidFlag;
};

} // namespace scag
} // namespace pvss
} // namespace pvap
#endif
