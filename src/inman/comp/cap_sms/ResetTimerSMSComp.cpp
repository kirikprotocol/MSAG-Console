#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/codec_inc/cap/ResetTimerSMSArg.h"
#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {

void SMSResetTimerArg::decode(const uint8_t * in_buf, uint16_t enc_len)
  throw(smsc::util::CustomException)
{
    ResetTimerSMSArg_t *dcmd = NULL;	/* decoded structure */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_ResetTimerSMSArg, (void **)&dcmd, in_buf, (size_t)enc_len);
    ASNCODEC_LOG_DEC(dcmd, drc, asn_DEF_ResetTimerSMSArg, "RSTSmsArg");

    tmrValue = dcmd->timervalue;

    smsc_log_component(compLogger, &asn_DEF_ResetTimerSMSArg, dcmd);
    asn_DEF_ResetTimerSMSArg.free_struct(&asn_DEF_ResetTimerSMSArg, dcmd, 0);
}

}//namespace comp
}//namespace inman
}//namespace smsc

