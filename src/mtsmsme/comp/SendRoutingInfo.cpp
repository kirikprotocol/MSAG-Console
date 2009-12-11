static char const ident[] = "$Id$";

#include "mtsmsme/comp/SendRoutingInfo.hpp"

namespace smsc{namespace mtsmsme{namespace comp{
SendRoutingInfoReq::SendRoutingInfoReq()
{
  char* msisdn="648237648723";
  ZERO_OCTET_STRING(_msisdn);
    _msisdn.size = (int)packNumString2BCD91(_msisdn.buf, msisdn, (unsigned)sizeof(msisdn));
  arg.msisdn = _msisdn;

  arg.cug_CheckInfo = 0;
  arg.numberOfForwarding = 0;
  arg.interrogationType = InterrogationType_basicCall;
  arg.or_Interrogation = 0;
  arg.or_Capability = 0;

  char* gmsc = "34563463";
  ZERO_OCTET_STRING(_gmsc);
    _gmsc.size = (int)packNumString2BCD91(_gmsc.buf, gmsc, (unsigned)sizeof(gmsc));
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
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
