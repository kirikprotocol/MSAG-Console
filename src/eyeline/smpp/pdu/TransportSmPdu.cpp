#include "TransportSmPdu.h"

namespace eyeline {
namespace smpp {


void TransportSmPdu::encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
{
    BaseSmPdu<ADDRLEN21>::encodeMandatoryFields(tb);
    tb.ncset8(esm_class);
    tb.ncset8(protocol_id);
    tb.ncset8(priority_flag);
    tb.setCString(schedule_delivery_time.c_str());
    tb.setCString(validity_period.c_str());
    tb.ncset8(registered_delivery);
    tb.ncset8(replace_if_present_flag);
    tb.ncset8(data_coding);
    tb.ncset8(sm_default_msg_id);
    tb.ncset8(sm_length);
    tb.copy(size_t(sm_length),short_message.str);
}


void TransportSmPdu::decodeMandatoryFields( eyeline::informer::FromBuf& fb )
{
    BaseSmPdu<ADDRLEN21>::decodeMandatoryFields(fb);

    esm_class = fb.get8();
    protocol_id = fb.get8();
    priority_flag = fb.get8();
    schedule_delivery_time = fb.getCString();
    validity_period = fb.getCString();

    registered_delivery = fb.get8();
    replace_if_present_flag = fb.get8();
    data_coding = fb.get8();
    sm_default_msg_id = fb.get8();
    const size_t sz = sm_length = fb.get8();
    short_message.assign(fb.skip(sz), sz);
}


const char* TransportSmPdu::toString( char* buf, size_t bsz, size_t* written ) const
{
    size_t wr;
    BaseSmPdu<ADDRLEN21>::toString(buf,bsz,&wr);
    bsz -= wr;
    if (bsz > 0) {
        int res = snprintf(buf+wr,bsz,",esm=%u,proto=%u,prio=%u,schtm=%s,val=%s,regdlv=%u,replfl=%u,dc=%u,smdflid=%u,smlen=%u",
                           esm_class,
                           protocol_id,
                           priority_flag,
                           schedule_delivery_time.c_str(),
                           validity_period.c_str(),
                           registered_delivery,
                           replace_if_present_flag,
                           data_coding,
                           sm_default_msg_id,
                           sm_length);
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
