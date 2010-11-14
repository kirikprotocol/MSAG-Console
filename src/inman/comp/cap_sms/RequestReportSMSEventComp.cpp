#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/codec_inc/cap/RequestReportSMSEventArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {

void SMSRequestReportEventArg::decode(const std::vector<unsigned char>& buf) throw(CustomException)
{
    RequestReportSMSEventArg_t *dcmd = 0;
    asn_dec_rval_t rval;

    rval = ber_decode(0, &asn_DEF_RequestReportSMSEventArg,(void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(dcmd, rval, asn_DEF_RequestReportSMSEventArg, "RRSmsEvtArg");

    const asn_anonymous_sequence_ * list = _A_CSEQUENCE_FROM_VOID(&dcmd->sMSEvents);
    for (int i = 0; i < list->count; i++) {
        SMSEvent_t *elem = static_cast<SMSEvent_t*>(list->array[i]);
        if (!elem)
            continue;
        events.insert(SMSEventDPs::value_type(
                            static_cast<EventTypeSMS_e>(elem->eventTypeSMS),
                            static_cast<MonitorMode_e>(elem->monitorMode)));
    }
    smsc_log_component(compLogger, &asn_DEF_RequestReportSMSEventArg, dcmd);
    asn_DEF_RequestReportSMSEventArg.free_struct(&asn_DEF_RequestReportSMSEventArg, dcmd, 0);
}

}//namespace comps
}//namespace inman
}//namespace smsc

