#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/codec_inc/cap/ConnectSMSArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {

void SMSConnectArg::decode(const std::vector<unsigned char>& buf) throw(CustomException)
{
    ConnectSMSArg_t *dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_ConnectSMSArg, (void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(dcmd, drc, asn_DEF_ConnectSMSArg, "ConnSmsArg");

    if (OCTET_STRING_2_Address(dcmd->destinationSubscriberNumber, dstSN))
        mask |= SMSConnectArg::connDSN;
    if (OCTET_STRING_2_Address(dcmd->callingPartysNumber, clngPN))
        mask |= SMSConnectArg::connCPN;
    if (OCTET_STRING_2_Address(dcmd->sMSCAddress, sMSCAdr))
        mask |= SMSConnectArg::connSMSC;

    smsc_log_component(compLogger, &asn_DEF_ConnectSMSArg, dcmd);
    asn_DEF_ConnectSMSArg.free_struct(&asn_DEF_ConnectSMSArg, dcmd, 0);
}

}//namespace comp
}//namespace inman
}//namespace smsc

