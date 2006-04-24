static char const ident[] = "$Id$";
#include <vector>

#include "inman/codec_inc/cap/ConnectSMSArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {
using std::vector;

ConnectSMSArg::ConnectSMSArg()
{
    compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.ConnectSMSArg");
}
ConnectSMSArg::~ConnectSMSArg() { }

void ConnectSMSArg::decode(const vector<unsigned char>& buf) throw(CustomException)
{
    ConnectSMSArg_t *dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_ConnectSMSArg, (void **)&dcmd, &buf[0], buf.size());
    INMAN_LOG_DEC(drc, asn_DEF_ConnectSMSArg);

    OCTET_STRING_2_Address(dcmd->destinationSubscriberNumber, dstSN);
    OCTET_STRING_2_Address(dcmd->callingPartysNumber, clngPN);
    OCTET_STRING_2_Address(dcmd->sMSCAddress, sMSCAdr);

    smsc_log_component(compLogger, &asn_DEF_ConnectSMSArg, dcmd);
    asn_DEF_ConnectSMSArg.free_struct(&asn_DEF_ConnectSMSArg, dcmd, 0);
}

}//namespace comp
}//namespace inman
}//namespace smsc

