static char const ident[] = "$Id$";
#include <vector>

#include "ResetTimerSMSArg.h"
#include "comps.hpp"
#include "compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {
using std::vector;


ResetTimerSMSArg::ResetTimerSMSArg()
{
    timerValue = 0; 
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.ResetTimerSMSArg");
}
ResetTimerSMSArg::~ResetTimerSMSArg() { }

void ResetTimerSMSArg::decode(const vector<unsigned char>& buf)
{
    ResetTimerSMSArg_t *dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */
    drc = ber_decode(0, &asn_DEF_ResetTimerSMSArg, (void **)&dcmd, &buf[0], buf.size());
    INMAN_LOG_DEC(drc, asn_DEF_ResetTimerSMSArg);

    timerValue = dcmd->timervalue;
    asn_DEF_ResetTimerSMSArg.free_struct(&asn_DEF_ResetTimerSMSArg, dcmd, 0);
}

/* this method doesn't required */
//void ResetTimerSMSArg::encode(vector<unsigned char>& buf)
//{ throw EncodeError("Not implemented"); }

}//namespace comp
}//namespace inman
}//namespace smsc

