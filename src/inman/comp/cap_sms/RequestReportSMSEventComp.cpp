static char const ident[] = "$Id$";

#include "inman/codec_inc/cap/RequestReportSMSEventArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {

RequestReportSMSEventArg::RequestReportSMSEventArg()
{
    compLogger = Logger::getInstance("smsc.inman.comp.RequestReportSMSEventArg");
}


void RequestReportSMSEventArg::decode(const std::vector<unsigned char>& buf) throw(CustomException)
{
    RequestReportSMSEventArg_t *req = 0;
    asn_dec_rval_t rval;

    rval = ber_decode(0, &asn_DEF_RequestReportSMSEventArg,(void **)&req, &buf[0], buf.size());
    INMAN_LOG_DEC(rval, asn_DEF_RequestReportSMSEventArg);

    const asn_anonymous_sequence_ * list = _A_CSEQUENCE_FROM_VOID(&req->sMSEvents);
    
    for (int i = 0; i < list->count; i++) {
        SMSEvent_t *elem = static_cast<SMSEvent_t*>(list->array[i]);
        if (!elem)
            continue;
        events.insert(SMSEventDPs::value_type(
                            static_cast<EventTypeSMS_e>(elem->eventTypeSMS),
                            static_cast<MonitorMode_e>(elem->monitorMode)));
    }
    smsc_log_component(compLogger, &asn_DEF_RequestReportSMSEventArg, req);
    asn_DEF_RequestReportSMSEventArg.free_struct(&asn_DEF_RequestReportSMSEventArg,req, 0);
}

}//namespace comps
}//namespace inman
}//namespace smsc

