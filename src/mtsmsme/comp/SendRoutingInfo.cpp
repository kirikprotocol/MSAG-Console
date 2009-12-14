static char const ident[] = "$Id$";

#include "mtsmsme/comp/SendRoutingInfo.hpp"

namespace smsc{namespace mtsmsme{namespace comp{
using smsc::mtsmsme::processor::util::packNumString2BCD91;
using smsc::mtsmsme::processor::util::unpackBCD912NumString;
using std::string;

SendRoutingInfoReq::SendRoutingInfoReq(const string& msisdn, const string& gmsc)
{
  ZERO_OCTET_STRING(_msisdn);
    _msisdn.size = (int)packNumString2BCD91(_msisdn.buf, msisdn.c_str(), (unsigned)msisdn.length());
  arg.msisdn = _msisdn;

  arg.cug_CheckInfo = 0;
  arg.numberOfForwarding = 0;
  arg.interrogationType = InterrogationType_basicCall;
  arg.or_Interrogation = 0;
  arg.or_Capability = 0;

  ZERO_OCTET_STRING(_gmsc);
    _gmsc.size = (int)packNumString2BCD91(_gmsc.buf, gmsc.c_str(), (unsigned)gmsc.length());
  arg.gmsc_OrGsmSCF_Address = _gmsc;

  arg.callReferenceNumber = 0;
  arg.forwardingReason = 0;
  arg.basicServiceGroup = 0;
  arg.networkSignalInfo = 0;
  arg.camelInfo = 0;
  arg.suppressionOfAnnouncement = 0;
  arg.extensionContainer = 0;
  arg.alertingPattern = 0;
  arg.ccbs_Call = 0;
  arg.supportedCCBS_Phase = 0;
  arg.additionalSignalInfo = 0;
  arg.istSupportIndicator = 0;
  arg.pre_pagingSupported = 0;
  arg.callDiversionTreatmentIndicator = 0;
  arg.longFTN_Supported = 0;
  arg.suppress_VT_CSI = 0;
  arg.suppressIncomingCallBarring = 0;
  arg.gsmSCF_InitiatedCall = 0;
  arg.basicServiceGroup2 = 0;
  arg.networkSignalInfo2 = 0;
}
void SendRoutingInfoReq::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_SendRoutingInfoArg, &arg, print2vec, &buf);
}
void SendRoutingInfoReq::decode(const vector<unsigned char>& buf)
{
  return;
}
char* SendRoutingInfoConf::getMSRN()
{
  return _msrn;
}
SendRoutingInfoConf::SendRoutingInfoConf(Logger* _logger) : logger(_logger)
{
  _msrn[0] = '0';
  _msrn[1] = '\0';
}
void SendRoutingInfoConf::encode(vector<unsigned char>& buf)
{
  return;
}
void SendRoutingInfoConf::decode(const vector<unsigned char>& buf)
{
  void *structure = 0;
  asn_codec_ctx_t s_codec_ctx;
  asn_codec_ctx_t *opt_codec_ctx = 0;
  opt_codec_ctx = &s_codec_ctx;
  asn_dec_rval_t rval;
  asn_TYPE_descriptor_t *def = &asn_DEF_SendRoutingInfoRes;

  rval = ber_decode(0/*opt_codec_ctx*/, def,
      (void **) &structure, (void *) &buf[0], buf.size());
  if (rval.code != RC_OK)
    smsc_log_error(logger,
        "SendRoutingInfoConf::decode consumes %d/%d and returns code %d",
        rval.consumed,buf.size(), rval.code)
      ;
  if (structure)
  {
    /* Invoke type-specific printer */
    std::vector<unsigned char> stream;
    def->print_struct(def, structure, 1, print2vec, &stream);
    /* Create and return resulting string */
    string result((char*) &stream[0], (char*) &stream[0] + stream.size());
    SendRoutingInfoRes_t& arg = *(SendRoutingInfoRes_t*) structure;
    if (arg.extendedRoutingInfo &&
        arg.extendedRoutingInfo->present == ExtendedRoutingInfo_PR_routingInfo)
    {
      RoutingInfo_t& ri = arg.extendedRoutingInfo->choice.routingInfo;
      if (ri.present == RoutingInfo_PR_roamingNumber)
      {
        OCTET_STRING_t& addr = ri.choice.roamingNumber;
        unpackBCD912NumString(_msrn, addr.buf, addr.size);
      }
    }
    smsc_log_debug(logger, "SendRoutingInfoConf\n%s\nmsrn=%s", result.c_str(),getMSRN());
    /* free decoded tree */
    def->free_struct(def, structure, 0);
  }
}
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
