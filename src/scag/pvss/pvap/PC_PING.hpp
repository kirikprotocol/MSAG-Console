// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_PING_HPP__
#define __SCAG_PVSS_PVAP_PC_PING_HPP__

#include "util/int.h"
#include <string>
#include "Exceptions.h"
#include "TypeId.h"


namespace scag{
namespace pvss{
namespace pvap{

// class PVAP;

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

    /*
    template <class DataStream> uint32_t length()const
    {
        uint32_t rv=0;
        return rv;
    }
     */


    template <class Proto, class DataStream>
        void serialize( const Proto& proto, DataStream& ds ) const throw (PvapException)
    {
        checkFields();
        // mandatory fields
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
