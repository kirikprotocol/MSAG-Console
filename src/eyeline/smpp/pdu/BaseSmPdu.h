#ifndef _EYELINE_SMPP_BASESMPDU_H
#define _EYELINE_SMPP_BASESMPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "PduAddr.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"


namespace eyeline {
namespace smpp {

template < size_t T >
class BaseSmPdu : public RequestPdu
{
protected: //mandatory fields
    smsc::core::buffers::FixedLengthString<6> service_type;
    addr_params_t<T> source_addr;
    addr_params_t<T> dest_addr;

public:
    BaseSmPdu( uint32_t thetype ) : RequestPdu(thetype) {
        source_addr.ton = 0;
        source_addr.npi = 0;
        dest_addr.ton = 0;
        dest_addr.npi = 0;
    }

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const
    {
        tb.setCString(service_type.c_str());
        source_addr.encode(tb);
        dest_addr.encode(tb);
    }
    void decodeMandatoryFields( eyeline::informer::FromBuf& fb )
    {
        service_type = fb.getCString();
        source_addr.decode(fb);
        dest_addr.decode(fb);
    }

    const char* toString( char* buf, size_t bsz, size_t* written = 0) const
    {
        size_t wr;
        RequestPdu::toString(buf,bsz,&wr);
        bsz -= wr;
        if (bsz > 0) {
            int res = snprintf(buf+wr,bsz,",svctyp=%s,src=.%u.%u.%s,dst=.%u.%u.%s",
                               service_type.c_str(),
                               source_addr.ton,
                               source_addr.npi,
                               source_addr.addr.c_str(),
                               dest_addr.ton,
                               dest_addr.npi,
                               dest_addr.addr.c_str());
            if (res<=0) {
                buf[wr] = '\0';
                res = 0;
            } else if ( size_t(res) >= bsz ) {
                res = int(bsz-1);
            }
            wr += res;
        }
        if (written) {
            *written = wr;
        }
        return buf;
    }

protected:
    size_t mandatorySize() const {
        return service_type.length() + 1 + source_addr.size() + dest_addr.size();
    }

protected:    //mandatory fields access
    inline const uint8_t getSourceAddrTon() const { return source_addr.ton; }
    inline const uint8_t getSourceAddrNpi() const { return source_addr.npi; }
    inline const char* getSourceAddr() const { return source_addr.addr.c_str(); }
    inline const uint8_t getDestAddrTon() const { return dest_addr.ton; }
    inline const uint8_t getDestAddrNpi() const { return dest_addr.npi; }
    inline const char* getDestAddr() const { return dest_addr.addr.c_str(); }

public:    //mandatory fields access
    inline const char* getServiceType() const { return service_type.c_str(); }
    void setServiceType(const char* value) { service_type = value; }

    void setSourceAddr(uint8_t ton, uint8_t npi, const char* value)
    {
        source_addr.ton = ton;
        source_addr.npi = npi;
        source_addr.addr = value;
    }

    void setSourceAddr(const char* value)
    {
        source_addr.setAddr(value);
    }

    void setDestAddr(uint8_t ton, uint8_t npi, const char* value)
    {
        dest_addr.ton = ton;
        dest_addr.npi = npi;
        dest_addr.addr = value;
    }

    void setDestAddr( const char* value )
    {
        dest_addr.setAddr(value);
    }
};

}
}

#endif
