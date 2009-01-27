// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_PING_HPP__
#define __SCAG_PVSS_PVAP_PC_PING_HPP__

#include "util/int.h"
#include <string>
#include "Exceptions.h"


namespace scag{
namespace pvss{
namespace pvap{

class PC_PING  
{
public:
    PC_PING()
    {
        clear();
    }
    void clear()
    {
    }

    std::string toString() const
    {
        std::string rv("PC_PING:");
        char buf[32];
        sprintf(buf,"seqNum=%d",seqNum);
        rv+=buf;
        return rv;
    }

    template <class DataStream> uint32_t length()const
    {
        uint32_t rv=0;
        return rv;
    }


    template <class DataStream> void serialize(DataStream& ds) const
    {
        checkFields();
        // mandatory fields
        // optional fields
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
        //  throw IncompatibleVersionException("PC_PING");
        //}
        //seqNum=ds.readInt32();
        while (!endOfMessage) {
            uint32_t tag=ds.readTag();
            switch(tag) {
            case DataStream::endOfMessage_tag:
                endOfMessage=true;
                break;
            default:
                //if(rdVersionMinor==versionMinor)
                //{
                //  throw UnexpectedTag("PC_PING",tag);
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
        // checking optional fields
    }

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;


    uint32_t seqNum;


};

} // namespace scag
} // namespace pvss
} // namespace pvap
#endif
