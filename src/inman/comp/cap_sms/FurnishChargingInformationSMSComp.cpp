#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/codec_inc/cap/FurnishChargingInformationSMSArg.h"
#include "inman/codec_inc/cap/CAMEL-FCISMSBillingChargingCharacteristics.h"

#include "inman/comp/cap_sms/CapSMSComps.hpp"
#include "inman/comp/compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {

void SMSFurnishChargingInformationArg::decode(const std::vector<unsigned char>& buf) throw(CustomException)
{
    FurnishChargingInformationSMSArg_t *dcmd = NULL;	/* decoded structure */
    CAMEL_FCISMSBillingChargingCharacteristics_t * dcmd2 = NULL; /* */
    asn_dec_rval_t	drc;		/* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_FurnishChargingInformationSMSArg, (void **)&dcmd,
                     &buf[0], buf.size());
    ASNCODEC_LOG_DEC(dcmd, drc, asn_DEF_FurnishChargingInformationSMSArg, "FCISmsArg");
    smsc_log_component(compLogger, &asn_DEF_FurnishChargingInformationSMSArg, dcmd);

    //dcmd keeps BER-encoded value of type CAMEL-FCISMSBillingChargingCharacteristics
    drc = ber_decode(0, &asn_DEF_CAMEL_FCISMSBillingChargingCharacteristics, (void **)&dcmd2,
                     dcmd->buf, dcmd->size);
    ASNCODEC_LOG_DEC(dcmd2, drc, asn_DEF_CAMEL_FCISMSBillingChargingCharacteristics, "FCISmsArg");
    smsc_log_component(compLogger, &asn_DEF_CAMEL_FCISMSBillingChargingCharacteristics, dcmd2);

    //to do: store content
    asn_DEF_FurnishChargingInformationSMSArg.free_struct(
            &asn_DEF_FurnishChargingInformationSMSArg, dcmd, 0);
    asn_DEF_CAMEL_FCISMSBillingChargingCharacteristics.free_struct(
            &asn_DEF_CAMEL_FCISMSBillingChargingCharacteristics, dcmd2, 0);
}

}//namespace comp
}//namespace inman
}//namespace smsc

