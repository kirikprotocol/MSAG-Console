static char const ident[] = "$Id$";
#include <vector>

#include "ConnectSMSArg.h"
#include "comps.hpp"
#include "compsutl.hpp"


namespace smsc {
namespace inman {
namespace comp {
using std::vector;

ConnectSMSArg::ConnectSMSArg()
{
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.ConnectSMSArg");
}
ConnectSMSArg::~ConnectSMSArg() { }

void ConnectSMSArg::decode(const vector<unsigned char>& buf)
{
    ConnectSMSArg_t *dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_ConnectSMSArg, (void **)&dcmd, &buf[0], buf.size());
    INMAN_LOG_DEC(drc, asn_DEF_ConnectSMSArg);

    dstSN  = OCTET_STRING_2_Addres(dcmd->destinationSubscriberNumber);
    clngPN = OCTET_STRING_2_Addres(dcmd->callingPartysNumber);
    sMSCAdr = OCTET_STRING_2_Addres(dcmd->sMSCAddress);

    smsc_log_component(compLogger, &asn_DEF_ConnectSMSArg, dcmd);
    asn_DEF_ConnectSMSArg.free_struct(&asn_DEF_ConnectSMSArg, dcmd, 0);
}

const Address & ConnectSMSArg::destinationSubscriberNumber() { return dstSN; }
const Address & ConnectSMSArg::callingPartyNumber() { return clngPN; }
const Address & ConnectSMSArg::SMSCAddress() { return sMSCAdr; }

/* this method doesn't required */
//void ConnectSMSArg::encode(vector<unsigned char>& buf) 
//{ throw EncodeError("Not implemented"); }

}//namespace comp
}//namespace inman
}//namespace smsc

