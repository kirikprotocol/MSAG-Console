static char const ident[] = "$Id$";

#include <vector>

#include "inman/codec_inc/cap/EventReportSMSArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"


namespace smsc {
namespace inman {
namespace comp {

using std::vector;

EventReportSMSArg::EventReportSMSArg(EventTypeSMS_e et, messageType_e mt)
		: eventType( et ), messageType( mt )
{
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.EventReportSMSArg");
}

void EventReportSMSArg::encode(vector<unsigned char>& buf) throw(CustomException)
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

