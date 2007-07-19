#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/codec_inc/cap/EventReportSMSArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {

EventReportSMSArg::EventReportSMSArg(EventTypeSMS_e et, messageType_e mt)
		: eventType( et ), messageType( mt )
{
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.EventReportSMSArg");
}

std::string & EventReportSMSArg::print(std::string & dump) const
{
    dump += "{";
    dump += _nmEventTypeSMS(eventType);
    dump += ", ";
    dump += _nmMessageType(messageType);
    dump += "}";
    return dump;
}

void EventReportSMSArg::encode(std::vector<unsigned char>& buf) throw(CustomException)
{
    asn_enc_rval_t	er;
    /* construct EventReportSMSArg */
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
    INMAN_LOG_ENC(er, asn_DEF_EventReportSMSArg);
}

}//namespace comp
}//namespace inman
}//namespace smsc

