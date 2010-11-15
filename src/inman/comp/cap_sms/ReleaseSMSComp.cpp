#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/codec_inc/cap/ReleaseSMSArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {

void SMSReleaseArg::decode(const std::vector<unsigned char>& buf) throw(CustomException)
{
    RPCause_t *		dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_ReleaseSMSArg, (void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(dcmd, drc, asn_DEF_ReleaseSMSArg, "RLSSmsArg");

    _rPCause = dcmd->buf[0];

    //NOTE: there is a deficiency in asn1c printing:
    //it doesn't print name for type defined via primitive type
    if (asn_DEF_ReleaseSMSArg.name) {
        smsc_log_debug(compLogger, "%s ::= ", asn_DEF_ReleaseSMSArg.name);
    }
    smsc_log_component(compLogger, &asn_DEF_ReleaseSMSArg, dcmd);
    asn_DEF_ReleaseSMSArg.free_struct(&asn_DEF_ReleaseSMSArg, dcmd, 0);
}

}//namespace comp
}//namespace inman
}//namespace smsc

