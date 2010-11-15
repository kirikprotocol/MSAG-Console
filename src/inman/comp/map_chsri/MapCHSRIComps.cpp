#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/codec_inc/map/SendRoutingInfoArg.h"
#include "inman/codec_inc/map/SendRoutingInfoRes.h"
#include "inman/comp/map_chsri/MapCHSRIComps.hpp"
#include "inman/comp/compsutl.hpp"
#include "inman/common/cvtutil.hpp"
#include "util/vformat.hpp"
using smsc::util::format;

using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::packNumString2BCD;
using smsc::cvtutil::packMAPAddress2OCTS;
using smsc::cvtutil::unpackBCD2NumString;
using smsc::inman::comp::OCTET_STRING_2_Address;
using smsc::inman::comp::smsc_log_component;


#define OCTET_STRING_DECL(name, szo) unsigned char name##_buf[szo]; OCTET_STRING_t name
#define ZERO_OCTET_STRING(name)	{ memset(&name, 0, sizeof(name)); name.buf = name##_buf; }

#define Address2OCTET_STRING(octs, addr)	{ ZERO_OCTET_STRING(octs); \
	octs.size = packMAPAddress2OCTS(addr, octs.buf); }

#define ZERO_VALUE(name)    memset(&name, 0, sizeof(name))

namespace smsc {
namespace inman {
namespace comp {
namespace chsri {

/* ************************************************************************** *
 * class CHSendRoutingInfoArg implementation:
 * ************************************************************************** */

//sets ISDN address of requesting point
void CHSendRoutingInfoArg::setGMSCorSCFaddress(const char * addr) throw(CustomException)
{
    if (!scfAdr.fromText(addr))
        throw CustomException(-1, "inalid scfAdr", addr);
}
//sets subscriber identity: IMSI or MSISDN addr
void CHSendRoutingInfoArg::setSubscrMSISDN(const char *addr) throw(CustomException)
{
    if (!subscrAdr.fromText(addr) || (subscrAdr.numPlanInd != 1))
        throw CustomException(-1, "inalid subscriber MSISDN", addr);
}

//ISDN BC IE for the preferred service,
//static unsigned char _si_buf[] = { 0x04, 0x03, 0x80, 0x90, 0xa3, 0x7d, 0x02, 0x91, 0x81 };

//3GPP TS 23.018 version 6.5.0 Release 6
//      7.2.2.1 Process SRI_HLR
void CHSendRoutingInfoArg::encode(std::vector<unsigned char>& buf) const throw(CustomException)
{
    asn_enc_rval_t          erc;
    SendRoutingInfoArg_t    cmd;
    unsigned char           isdn_buf[sizeof(TONNPI_ADDRESS_OCTS)];
    unsigned char           isdn_buf2[sizeof(TONNPI_ADDRESS_OCTS)];

    ZERO_VALUE(cmd); //clear asn_ctx & optionals
    cmd.interrogationType = InterrogationType_basicCall;

    cmd.msisdn.buf = isdn_buf;
    cmd.msisdn.size = packMAPAddress2OCTS(subscrAdr, isdn_buf);

    cmd.gmsc_OrGsmSCF_Address.buf = isdn_buf2;
    cmd.gmsc_OrGsmSCF_Address.size = packMAPAddress2OCTS(scfAdr, isdn_buf2);
/*
    CallReferenceNumber_t   crfn;
    unsigned char           crfn_buf = 0x0F; //SIZE(1..8)
    ZERO_VALUE(crfn); //clear asn_ctx & optionals
    crfn.buf = &crfn_buf;
    crfn.size = 1;
    cmd.callReferenceNumber = &crfn;
*/
/*
    ExternalSignalInfo_t    nsi;
    ZERO_VALUE(nsi); //clear asn_ctx & optionals
    nsi.protocolId = ProtocolId_ets_300102_1;
    nsi.signalInfo.buf = _si_buf;
    nsi.signalInfo.size = sizeof(_si_buf);
    cmd.networkSignalInfo = &nsi;
*/
    CamelInfo_t     ci;
    unsigned char   cph_buf = 0xE0; //'1110 0000': phase1 - phase3
    ZERO_VALUE(ci); //clear asn_ctx & optionals
    ci.supportedCamelPhases.buf = &cph_buf;
    ci.supportedCamelPhases.size = 1;
    ci.supportedCamelPhases.bits_unused = 5;
    cmd.camelInfo = &ci;

//    SupportedCCBS_Phase_t   ccbs = 1;
//    cmd.supportedCCBS_Phase = &ccbs;

    smsc_log_component(compLogger, &asn_DEF_SendRoutingInfoArg, &cmd);
    erc = der_encode(&asn_DEF_SendRoutingInfoArg, &cmd, print2vec, &buf);
    ASNCODEC_LOG_ENC(erc, asn_DEF_SendRoutingInfoArg, "SRIArg");
    return;
}


/* ************************************************************************** *
 * class CHSendRoutingInfoRes implementation:
 * ************************************************************************** */

static bool parse_O_CSI(O_CSI_t *csi, GsmSCFinfo *scf_inf) throw(CustomException)
{
    if (csi->o_BcsmCamelTDPDataList.list.count) {
        O_BcsmCamelTDPData_t *root_elem = csi->o_BcsmCamelTDPDataList.list.array[0];
        scf_inf->serviceKey = (uint32_t)root_elem->serviceKey;
           
        if (!OCTET_STRING_2_Address(&(root_elem->gsmSCF_Address), scf_inf->scfAddress))
            throw CustomException(-11, "O_CSI: bad gsmSCF_Adr", NULL);

        for (int i = 1; i < csi->o_BcsmCamelTDPDataList.list.count; i++) {
            O_BcsmCamelTDPData_t *elem = csi->o_BcsmCamelTDPDataList.list.array[i];
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

unsigned short CHSendRoutingInfoRes::getSCFinfo(GsmSCFinfo * scf_dat) const
{
    if (!mask.st.o_csi)
        return 0;
    *scf_dat = o_csi;
    return mask.st.o_csi + mask.st.imsi;
}

unsigned short CHSendRoutingInfoRes::getIMSI(IMSIString & out_imsi) const
{
    if (mask.st.imsi)
      out_imsi = o_imsi;
    return mask.st.imsi;
}

unsigned short CHSendRoutingInfoRes::getVLRN(TonNpiAddress & vlr_n) const
{
    if (mask.st.n_vlr)
        vlr_n = vlrNum;
    return mask.st.n_vlr;
}

void CHSendRoutingInfoRes::decode(const std::vector<unsigned char>& buf) throw(CustomException)
{
    SendRoutingInfoRes_t *  dcmd = NULL;
    asn_dec_rval_t  drc;    /* Decoder return value  */

    drc = ber_decode(0, &asn_DEF_SendRoutingInfoRes, (void **)&dcmd, &buf[0], buf.size());
    ASNCODEC_LOG_DEC(dcmd, drc, asn_DEF_SendRoutingInfoRes, "SRIRes");
    smsc_log_component(compLogger, &asn_DEF_SendRoutingInfoRes, dcmd);

    try {
        if (dcmd->imsi) {
            if (dcmd->imsi->size >= o_imsi.capacity()
                || !unpackBCD2NumString(dcmd->imsi->buf, o_imsi.str, dcmd->imsi->size))
                throw CustomException(-1, "SRIRes: bad IMSI");
            mask.st.imsi = 1;
        }
        if (dcmd->extendedRoutingInfo
            && (dcmd->extendedRoutingInfo->present == ExtendedRoutingInfo_PR_camelRoutingInfo)) {
            O_CSI_t * gCSI = 
                dcmd->extendedRoutingInfo->choice.camelRoutingInfo.gmscCamelSubscriptionInfo.o_CSI;
            if (gCSI)
                mask.st.o_csi = parse_O_CSI(gCSI, &o_csi) ? 1 : 0;
        }
        ISDN_AddressString_t * vlr_os = NULL;
        if (dcmd->subscriberInfo && dcmd->subscriberInfo->locationInformation
            && (vlr_os = dcmd->subscriberInfo->locationInformation->vlr_number)) {
            if (!OCTET_STRING_2_Address(vlr_os, vlrNum))
                throw CustomException(-1, "SRIRes: bad vlr_number");
            mask.st.n_vlr = 1;
        }
        asn_DEF_SendRoutingInfoRes.free_struct(&asn_DEF_SendRoutingInfoRes, dcmd, 0);
    } catch (const CustomException & exc) {
        asn_DEF_SendRoutingInfoRes.free_struct(&asn_DEF_SendRoutingInfoRes, dcmd, 0);
        throw;
    }
    return;
}


void CHSendRoutingInfoRes::mergeSegment(Component * segm) throw(CustomException)
{
    if (!segm)
        return;

    CHSendRoutingInfoRes *  sriNL = static_cast<CHSendRoutingInfoRes*>(segm);
    if (sriNL->hasIMSI()) {
        sriNL->getIMSI(o_imsi);
        mask.st.imsi = 1;
    }
    if (sriNL->hasOCSI()) {
        mask.st.o_csi = 1;
        sriNL->getSCFinfo(&o_csi);
    }
    if (sriNL->hasVLRN()) {
        mask.st.n_vlr = 1;
        sriNL->getVLRN(vlrNum);
    }
}


}//namespace chsri
}//namespace comp
}//namespace inman
}//namespace smsc

