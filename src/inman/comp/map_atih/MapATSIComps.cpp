#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/codecs/map_atih/AnyTimeSubscriptionInterrogationArg.h"
#include "inman/codecs/map_atih/AnyTimeSubscriptionInterrogationRes.h"
#include "inman/comp/map_atih/MapATSIComps.hpp"
#include "inman/comp/compsutl.hpp"
#include "inman/common/cvtutil.hpp"
#include "util/vformat.hpp"

using smsc::util::format;
using smsc::util::MAPConst;

using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::packNumString2BCD;
using smsc::cvtutil::packMAPAddress2OCTS;
using smsc::inman::comp::OCTET_STRING_2_Address;
using smsc::inman::comp::smsc_log_component;


#define OCTET_STRING_DECL(name, szo) unsigned char name##_buf[szo]; OCTET_STRING_t name
#define ZERO_OCTET_STRING(name)	{ memset(&name, 0, sizeof(name)); name.buf = name##_buf; }

#define Address2OCTET_STRING(octs, addr)	{ ZERO_OCTET_STRING(octs); \
	octs.size = packMAPAddress2OCTS(addr, octs.buf); }

namespace smsc {
namespace inman {
namespace comp {
namespace atih {

/* ************************************************************************** *
 * class ATSIArg implementation:
 * ************************************************************************** */
//sets ISDN address of requesting point
void ATSIArg::setSCFaddress(const char * addr) throw(CustomException)
{
    if (!scfAdr.fromText(addr))
        throw CustomException(-1, "inalid scfAdr", addr);
}
//sets subscriber identity: IMSI or MSISDN addr
void ATSIArg::setSubscriberId(const char *addr, bool imsi/* = true*/) throw(CustomException)
{
    if (!subscrAdr.fromText(addr))
        throw CustomException(-1, "inalid subscriberID", addr);
    if ((subscrImsi = imsi)) {
        if (((subscrAdr.length + 1)/2) > MAPConst::MAX_IMSI_AddressLength)
            throw CustomException("IMSI length is too long: %u", subscrAdr.length);
    }
}

void ATSIArg::encode(std::vector<unsigned char>& buf) const throw(CustomException)
{
    asn_enc_rval_t                          erc;
    AnyTimeSubscriptionInterrogationArg_t   cmd;
    RequestedCAMEL_SubscriptionInfo_t	    reqCSI = askCSI;
    unsigned char                           imsi_buf[MAPConst::MAX_IMSI_AddressLength];
    unsigned char                           isdn_buf[sizeof(TONNPI_ADDRESS_OCTS)];
    unsigned char                           isdn_buf2[sizeof(TONNPI_ADDRESS_OCTS)];

    memset(&cmd, 0, sizeof(cmd)); //clear asn_ctx & optionals
    cmd.subscriberIdentity.present = subscrImsi ? SubscriberIdentity_PR_imsi
                                                : SubscriberIdentity_PR_msisdn;
    if (subscrImsi) {   //TBCD string
        memset(&imsi_buf, 0, sizeof(imsi_buf)); 
        cmd.subscriberIdentity.choice.imsi.size = 
            packNumString2BCD(imsi_buf, subscrAdr.getSignals(), subscrAdr.length);
        cmd.subscriberIdentity.choice.imsi.buf = imsi_buf;
    } else {            //ISDNAddress
        cmd.subscriberIdentity.choice.msisdn.buf = isdn_buf;
        cmd.subscriberIdentity.choice.msisdn.size = packMAPAddress2OCTS(subscrAdr, isdn_buf);
    }
    cmd.requestedSubscriptionInfo.requestedCAMEL_SubscriptionInfo = &reqCSI;

    cmd.gsmSCF_Address.buf = isdn_buf2;
    cmd.gsmSCF_Address.size = packMAPAddress2OCTS(scfAdr, isdn_buf2);

    smsc_log_component(compLogger, &asn_DEF_AnyTimeSubscriptionInterrogationArg, &cmd);
    erc = der_encode(&asn_DEF_AnyTimeSubscriptionInterrogationArg, &cmd, print2vec, &buf);
    ASNCODEC_LOG_ENC(erc, asn_DEF_AnyTimeSubscriptionInterrogationArg, "mapATIH");
    return;
}


/* ************************************************************************** *
 * class ATSIRes implementation:
 * ************************************************************************** */
static bool parse_O_CSI(CAMEL_SubscriptionInfo_t *cs, GsmSCFinfo *scf_inf) throw(CustomException)
{
    if (cs->o_CSI && cs->o_CSI->csiActive 
        && cs->o_CSI->o_BcsmCamelTDPDataList.list.count) {

        O_BcsmCamelTDPData_t *root_elem = cs->o_CSI->o_BcsmCamelTDPDataList.list.array[0];
        scf_inf->serviceKey = (uint32_t)root_elem->serviceKey;
           
        if (!OCTET_STRING_2_Address(&(root_elem->gsmSCF_Address), scf_inf->scfAddress))
            throw CustomException(-11, "O_CSI: bad gsmSCF_Adr", NULL);

        for (int i = 1; i < cs->o_CSI->o_BcsmCamelTDPDataList.list.count; i++) {
            O_BcsmCamelTDPData_t *elem = cs->o_CSI->o_BcsmCamelTDPDataList.list.array[i];
            if ((root_elem->gsmSCF_Address.size != elem->gsmSCF_Address.size)
                || memcmp(root_elem->gsmSCF_Address.buf,
                          elem->gsmSCF_Address.buf, root_elem->gsmSCF_Address.size)) {
                std::string msg;
                format(msg, "element: %u, trigger: %u", i, elem->o_BcsmTriggerDetectionPoint);
                throw CustomException(-12, "O_CSI: different gsmSCF_Adr", msg.c_str());
            }
        }
        return true;
    }
    return false;
}

bool ATSIRes::getSCFinfo(enum RequestedCAMEL_SubscriptionInfo req_csi, GsmSCFinfo * scf_dat) const
{
    if (!(mask & (1 << req_csi)))
        return false;

    switch (req_csi) {
    case RequestedCAMEL_SubscriptionInfo_o_CSI:
        *scf_dat = o_csi;
        return true;
    default:;
    }
    return false;
}

void ATSIRes::decode(const std::vector<unsigned char>& buf) throw(CustomException)
{
    AnyTimeSubscriptionInterrogationRes *  dcmd = NULL;  /* decoded structure */
    asn_dec_rval_t  drc;    /* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_AnyTimeSubscriptionInterrogationRes, (void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(dcmd, drc, asn_DEF_AnyTimeSubscriptionInterrogationRes, "ATSIRes");
    smsc_log_component(compLogger, &asn_DEF_AnyTimeSubscriptionInterrogationRes, dcmd);

    try {
        if (!dcmd->camel_SubscriptionInfo)
            throw CustomException(-1, "ATSIRes: CSI missing");

        if (parse_O_CSI(dcmd->camel_SubscriptionInfo, &o_csi))
            mask |= (1 << RequestedCAMEL_SubscriptionInfo_o_CSI);
        
        asn_DEF_AnyTimeSubscriptionInterrogationRes.free_struct(&asn_DEF_AnyTimeSubscriptionInterrogationRes, dcmd, 0);
    } catch (CustomException & exc) {
        asn_DEF_AnyTimeSubscriptionInterrogationRes.free_struct(&asn_DEF_AnyTimeSubscriptionInterrogationRes, dcmd, 0);
        throw;
    }
    return;
}


}//namespace atih
}//namespace comp
}//namespace inman
}//namespace smsc
