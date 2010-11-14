#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/codec_inc/cap/EventReportSMSArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {

std::string & SMSEventReportArg::print(std::string & dump) const
{
    dump += "{";
    dump += _nmEventTypeSMS(eventType);
    dump += ", ";
    dump += _nmMessageType(messageType);
    dump += "}";
    return dump;
}

void SMSEventReportArg::encode(std::vector<unsigned char>& buf) const throw(CustomException)
{
    asn_enc_rval_t	er;
    /* construct SMSEventReportArg */
    EventReportSMSArg_t	erp;
    MiscCallInfo_t	mcs;

    memset(&erp, 0x00, sizeof(erp));
    memset(&mcs, 0x00, sizeof(mcs));
    erp.miscCallInfo = &mcs;
    /**/
    mcs.messageType = messageType;
    erp.eventTypeSMS = eventType;

    smsc_log_component(compLogger, &asn_DEF_EventReportSMSArg, &erp); 
    er = der_encode(&asn_DEF_EventReportSMSArg, &erp, print2vec, &buf);
    ASNCODEC_LOG_ENC(er, asn_DEF_EventReportSMSArg, "ERSmsArg");
}

}//namespace comp
}//namespace inman
}//namespace smsc

