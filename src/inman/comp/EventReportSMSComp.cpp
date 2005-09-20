static char const ident[] = "$Id$";
#include <vector>

#include "inman/codec/EventReportSMSArg.h"
#include "comps.hpp"
#include "compsutl.hpp"


namespace smsc {
namespace inman {
namespace comp {

using std::vector;

EventReportSMSArg::EventReportSMSArg(EventTypeSMS_e et, messageType_e mt)
		: eventType( et ), messageType( mt )
{
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.EventReportSMSArg");
    smsc_log_debug( compLogger, "EventReportSMSArg(%d,%d)", et, mt );
}

//EventReportSMSArg::~EventReportSMSArg() {}

/* this method doesn't required */
//void EventReportSMSArg::decode(const vector<unsigned char>& buf)
//{ throw DecodeError("Not implemented"); }

void EventReportSMSArg::encode(vector<unsigned char>& buf)
{
	smsc_log_debug( compLogger, "EventReportSMSArg enter");
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

    er = der_encode(&asn_DEF_EventReportSMSArg, &erp, print2vec, &buf);
    INMAN_LOG_ENC(er, asn_DEF_EventReportSMSArg);
    smsc_log_debug( compLogger, "EventReportSMSArg exit");
}

}//namespace comp
}//namespace inman
}//namespace smsc

