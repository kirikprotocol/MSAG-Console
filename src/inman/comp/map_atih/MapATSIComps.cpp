#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/codecs/map_atih/AnyTimeSubscriptionInterrogationArg.h"
#include "inman/codecs/map_atih/AnyTimeSubscriptionInterrogationRes.h"
#include "inman/comp/map_atih/MapATSIComps.hpp"
#include "inman/common/adrutil.hpp"
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
    throw CustomException(-1, "ATSIArg: inalid scfAdr", addr);
}
//sets subscriber identity: IMSI or MSISDN addr
void ATSIArg::setSubscriberId(const char *addr, bool imsi/* = true*/) throw(CustomException)
{
  if (!subscrAdr.fromText(addr))
    throw CustomException(-1, "ATSIArg: inalid subscriberID", addr);
  if ((subscrImsi = imsi)) {
    if ((unsigned)((subscrAdr.length + 1)/2) > MAPConst::MAX_IMSI_AddressLength)
      throw CustomException("ATSIArg: IMSI length is too long: %u", subscrAdr.length);
  }
}

void ATSIArg::encode(std::vector<unsigned char>& buf) const throw(CustomException)
{
  if (!getFlagsValue())
    throw CustomException(-2, "ATSIArg: Requested subscription isn't set");

  asn_enc_rval_t                          erc;
  AnyTimeSubscriptionInterrogationArg_t   cmd;
  RequestedCAMEL_SubscriptionInfo_t	    reqCSI;
  AdditionalRequestedCAMEL_SubscriptionInfo_t reqXCSI;
  SS_ForBS_Code_t                         reqSS;
  BasicServiceCode_t                      bsCode;
  unsigned char                           ssCode_buf[1];
  unsigned char                           bsCode_buf[1];
  unsigned char                           imsi_buf[MAPConst::MAX_IMSI_AddressLength];
  unsigned char                           isdn_buf[sizeof(TONNPI_ADDRESS_OCTS)];
  unsigned char                           isdn_buf2[sizeof(TONNPI_ADDRESS_OCTS)];
  NULL_t                                  present = 1;

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
  if (getFlags().odb)
    cmd.requestedSubscriptionInfo.odb = &present;
  if (getFlags().vlr)
    cmd.requestedSubscriptionInfo.supportedVLR_CAMEL_Phases = &present;
  if (getFlags().sgsn)
    cmd.requestedSubscriptionInfo.supportedSGSN_CAMEL_Phases = &present;

  if (getFlags().csi) {
    reqCSI = getBasicCSI();
    cmd.requestedSubscriptionInfo.requestedCAMEL_SubscriptionInfo = &reqCSI;
  }
  if (getFlags().xcsi) {
    reqXCSI = getAdditionalCSI();
    cmd.requestedSubscriptionInfo.additionalRequestedCAMEL_SubscriptionInfo = &reqXCSI;
  }
  if (getFlags().ss) {
    memset(&reqSS, 0, sizeof(reqSS)); //clear asn_ctx & optionals

    ssCode_buf[0] = getRequestedSSInfo().ssCode;
    reqSS.ss_Code.buf = ssCode_buf;
    reqSS.ss_Code.size = 1;

    if (getRequestedSSInfo().bsCodeKind != BasicServiceCode_PR_NOTHING) {
      memset(&bsCode, 0, sizeof(bsCode)); //clear asn_ctx & optionals

      bsCode.present = getRequestedSSInfo().bsCodeKind;
      bsCode_buf[0] = getRequestedSSInfo().bsCode.bearer;
      bsCode.choice.bearerService.buf = bsCode_buf;
      bsCode.choice.bearerService.size = 1;

      reqSS.basicService = &bsCode;
    }
  }

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
static bool parse_O_CSI(O_CSI * cs, GsmSCFinfo & scf_inf, Logger * use_logger) /*throw()*/
{
  scf_inf.Reset();

  for (int i = 0; i < cs->o_BcsmCamelTDPDataList.list.count; ++i) {
    O_BcsmCamelTDPData_t * elem = cs->o_BcsmCamelTDPDataList.list.array[i];

    if (elem->o_BcsmTriggerDetectionPoint == O_BcsmTriggerDetectionPoint_collectedInfo) {
      if (!OCTET_STRING_2_Address(&(elem->gsmSCF_Address), scf_inf.scfAddress)) {
        scf_inf.Reset();
        smsc_log_error(use_logger, "O_CSI: corrupted gsmSCF_Address");
      } else {
        scf_inf.serviceKey = (uint32_t)elem->serviceKey;
        return true;
      }
    }
  }
  smsc_log_error(use_logger, "T_CSI: missing tDP = collectedInfo (%u)",
                 O_BcsmTriggerDetectionPoint_collectedInfo);
  return false;
}

static bool parse_T_CSI(T_CSI * cs, GsmSCFinfo & scf_inf, Logger * use_logger) /*throw()*/
{
  scf_inf.Reset();

  for (int i = 0; i < cs->t_BcsmCamelTDPDataList.list.count; ++i) {
    T_BcsmCamelTDPData_t * elem = cs->t_BcsmCamelTDPDataList.list.array[i];

    if (elem->t_BcsmTriggerDetectionPoint == T_BcsmTriggerDetectionPoint_termAttemptAuthorized) {
      if (!OCTET_STRING_2_Address(&(elem->gsmSCF_Address), scf_inf.scfAddress)) {
        scf_inf.Reset();
        smsc_log_error(use_logger, "T_CSI: corrupted gsmSCF_Address");
      } else {
        scf_inf.serviceKey = (uint32_t)elem->serviceKey;
        return true;
      }
    }
  }
  smsc_log_error(use_logger, "T_CSI: missing tDP = termAttemptAuthorized (%u)",
                 T_BcsmTriggerDetectionPoint_termAttemptAuthorized);
  return false;
}

struct SMS_TDP {
  enum Mode_e { smsMO = 0, smsMT = 1 };

  const Mode_e  mode;
  const char *  nmMode;
  const char *  nmTDP;
  const SMS_TriggerDetectionPoint_t   val;

  explicit SMS_TDP(Mode_e use_mode) : mode(use_mode)
    , nmMode(mode ? "T" : "O"), nmTDP(mode ? "sms_DeliveryRequest" : "sms_CollectedInfo")
    , val(mode ? SMS_TriggerDetectionPoint_sms_DeliveryRequest
               : SMS_TriggerDetectionPoint_sms_CollectedInfo)
  { }
};

static const SMS_TDP    _moSmsTDP(SMS_TDP::smsMO);
static const SMS_TDP    _mtSmsTDP(SMS_TDP::smsMT);

static bool parse_SM_CSI(SMS_CSI * cs, GsmSCFinfo & scf_inf, const SMS_TDP & use_tdp,
                          Logger * use_logger) /*throw()*/
{
  scf_inf.Reset();
  
  if (cs->sms_CAMEL_TDP_DataList) {
    for (int i = 0; i < cs->sms_CAMEL_TDP_DataList->list.count; ++i) {
      SMS_CAMEL_TDP_Data * elem = cs->sms_CAMEL_TDP_DataList->list.array[i];

      if (elem->sms_TriggerDetectionPoint == use_tdp.val) {
        if (!OCTET_STRING_2_Address(&(elem->gsmSCF_Address), scf_inf.scfAddress)) {
          scf_inf.Reset();
          smsc_log_error(use_logger, "%s_SM_CSI: corrupted gsmSCF_Address", use_tdp.nmMode);
        } else {
          scf_inf.serviceKey = (uint32_t)elem->serviceKey;
          return true;
        }
      }
    }
  }
  smsc_log_error(use_logger, "$s_SM_CSI: missing tDP = %s (%u)", use_tdp.nmMode,
                 use_tdp.nmTDP, use_tdp.val);
  return false;
}


void ATSIRes::decode(const std::vector<unsigned char>& buf) throw(CustomException)
{
  AnyTimeSubscriptionInterrogationRes *  dcmd = NULL;  /* decoded structure */
  asn_dec_rval_t  drc;    /* Decoder return value  */

  drc = ber_decode(0, &asn_DEF_AnyTimeSubscriptionInterrogationRes, (void **)&dcmd, &buf[0], buf.size());
  ASNCODEC_LOG_DEC(dcmd, drc, asn_DEF_AnyTimeSubscriptionInterrogationRes, "ATSIRes");
  smsc_log_component(compLogger, &asn_DEF_AnyTimeSubscriptionInterrogationRes, dcmd);

  _csInfo.clear();
  _scfCsi.clear();
  _odbGD.clear();
  try { //for now process only camel_SubscriptionInfo for BC and SM
    CAMEL_SubscriptionInfo_t * cs = dcmd->camel_SubscriptionInfo;
    if (cs) {
      //Basic Call CSIs
      if (cs->o_CSI && cs->o_CSI->csiActive
          && parse_O_CSI(cs->o_CSI, _scfCsi[UnifiedCSI::csi_O_BC], compLogger))
        _csInfo.setCSI(UnifiedCSI::csi_O_BC);

      if (cs->t_CSI && cs->t_CSI->csi_Active
          && parse_T_CSI(cs->t_CSI, _scfCsi[UnifiedCSI::csi_T_BC], compLogger))
        _csInfo.setCSI(UnifiedCSI::csi_T_BC);

      //SMs CSIs
      if (cs->mo_sms_CSI && cs->mo_sms_CSI->csi_Active
          && parse_SM_CSI(cs->mo_sms_CSI, _scfCsi[UnifiedCSI::csi_MO_SM],
                          _moSmsTDP, compLogger))
        _csInfo.setCSI(UnifiedCSI::csi_MO_SM);

      if (cs->mt_sms_CSI && cs->mt_sms_CSI->csi_Active
          && parse_SM_CSI(cs->mt_sms_CSI, _scfCsi[UnifiedCSI::csi_MT_SM],
                          _mtSmsTDP, compLogger))
        _csInfo.setCSI(UnifiedCSI::csi_MT_SM);
    }
    //process ODB-GenaralData only
    if (dcmd->odb_Info && dcmd->odb_Info->odb_Data.odb_GeneralData.size) {
      ODB_GeneralData_t & dOdb = dcmd->odb_Info->odb_Data.odb_GeneralData;
      _odbGD.init(dOdb.buf, (uint16_t)(dOdb.size * 8 - dOdb.bits_unused));
      _csInfo.setRequestedODB();
    }
    asn_DEF_AnyTimeSubscriptionInterrogationRes.free_struct(&asn_DEF_AnyTimeSubscriptionInterrogationRes, dcmd, 0);
  } catch (const CustomException & exc) {
    asn_DEF_AnyTimeSubscriptionInterrogationRes.free_struct(&asn_DEF_AnyTimeSubscriptionInterrogationRes, dcmd, 0);
    throw;
  }
  return;
}


}//namespace atih
}//namespace comp
}//namespace inman
}//namespace smsc
