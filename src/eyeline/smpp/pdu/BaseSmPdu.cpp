/*
#include "BaseSmPdu.h"

namespace eyeline {
namespace smpp {

template < size_t T >
void BaseSmPdu<T>::encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const
{
    tb.setCString(service_type.c_str());
    tb.ncset8(source_addr.ton);
    tb.ncset8(source_addr.npi);
    tb.setCString(source_addr.addr.c_str());
    tb.ncset8(dest_addr.ton);
    tb.ncset8(dest_addr.npi);
    tb.setCString(dest_addr.addr.c_str());
}


template < size_t T >
void BaseSmPdu<T>::decodeMandatoryFields( eyeline::informer::FromBuf& fb )
{
    service_type = fb.getCString();
    source_addr.ton = fb.get8();
    source_addr.npi = fb.get8();
    source_addr.addr = fb.getCString();
    dest_addr.ton = fb.get8();
    dest_addr.npi = fb.get8();
    dest_addr.addr = fb.getCString();
}

template < size_t T >
const char* BaseSmPdu<T>::toString( char* buf, size_t bsz, size_t* written) const
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

}
}
*/
