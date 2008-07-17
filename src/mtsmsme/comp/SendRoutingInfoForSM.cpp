static char const ident[] = "$Id$";

#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "mtsmsme/processor/util.hpp"
#include "logger/Logger.h"
namespace smsc{namespace mtsmsme{namespace comp{

using std::vector;
using std::string;
using smsc::mtsmsme::processor::util::packNumString2BCD;
using smsc::mtsmsme::processor::util::packNumString2BCD91;
using smsc::mtsmsme::processor::util::unpackBCD912NumString;

SendRoutingInfoForSMReq::SendRoutingInfoForSMReq(const string& msisdn, bool force, const string& sca )
{
  ZERO_OCTET_STRING(_msisdn);
  _msisdn.size = packNumString2BCD91(_msisdn.buf, msisdn.c_str(), msisdn.length());
  arg.msisdn = _msisdn;

  ZERO_OCTET_STRING(_sca);
  _sca.size = packNumString2BCD91(_sca.buf, sca.c_str(), sca.length());
  arg.serviceCentreAddress = _sca;

  arg.sm_RP_PRI = force;
  arg.extensionContainer = 0;
  arg.gprsSupportIndicator = 0;
  arg.sm_RP_MTI = 0;
  arg.sm_RP_SMEA = 0;
}

SendRoutingInfoForSMReq::~SendRoutingInfoForSMReq()
{
}

void SendRoutingInfoForSMReq::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_RoutingInfoForSM_Arg, &arg, print2vec, &buf);
}
void SendRoutingInfoForSMReq::decode(const vector<unsigned char>& buf)
{
  return;
}
SendRoutingInfoForSMResp::SendRoutingInfoForSMResp(const string& imsi, const string& msc)
{
  ZERO_OCTET_STRING(_imsi);
  _imsi.size = packNumString2BCD91(_imsi.buf, imsi.c_str(), imsi.length());
  res.imsi = _imsi;

  ZERO_OCTET_STRING(_msc);
  _msc.size = packNumString2BCD91(_msc.buf, msc.c_str(), msc.length());
  res.locationInfoWithLMSI.networkNode_Number = _msc;

  res.locationInfoWithLMSI.lmsi = 0;
  res.locationInfoWithLMSI.extensionContainer = 0;
  res.locationInfoWithLMSI.gprsNodeIndicator = 0;
  res.locationInfoWithLMSI.additional_Number = 0;
  res.extensionContainer = 0;
}
SendRoutingInfoForSMResp::~SendRoutingInfoForSMResp()
{

}
void SendRoutingInfoForSMResp::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_RoutingInfoForSM_Res, &res, print2vec, &buf);
}
void SendRoutingInfoForSMResp::decode(const vector<unsigned char>& buf)
{

}
SendRoutingInfoForSMInd::SendRoutingInfoForSMInd(Logger* _logger) :
  logger(_logger)
{
  _msisdn[0] = '0';
  _msisdn[1] = '\0';
}
char* SendRoutingInfoForSMInd::getMSISDN()
{
 return _msisdn;
}
void SendRoutingInfoForSMInd::encode(vector<unsigned char>& buf)
{
  return;
}
void SendRoutingInfoForSMInd::decode(const vector<unsigned char>& buf)
{
  void *structure = 0;
  asn_codec_ctx_t s_codec_ctx;
  asn_codec_ctx_t *opt_codec_ctx = 0;
  opt_codec_ctx = &s_codec_ctx;
  asn_dec_rval_t rval;
  asn_TYPE_descriptor_t *def = &asn_DEF_RoutingInfoForSM_Arg;

  rval = ber_decode(0/*opt_codec_ctx*/, &asn_DEF_RoutingInfoForSM_Arg,
      (void **) &structure, (void *)&buf[0], buf.size());

  smsc_log_debug(logger,
      "SendRoutingInfoForSMInd::decode consumes %d/%d and returns code %d", rval.consumed,
      buf.size(), rval.code);
  if (structure)
  {
    /* Invoke type-specific printer */
    std::vector<unsigned char> stream;
    def->print_struct(def, structure, 1, print2vec, &stream);
    /* Create and return resulting string */
    string result((char*)&stream[0],(char*)&stream[0]+stream.size());
    RoutingInfoForSM_Arg_t& arg = *(RoutingInfoForSM_Arg_t*)structure;
    unpackBCD912NumString(_msisdn,arg.msisdn.buf,arg.msisdn.size);
    smsc_log_debug(logger, "SendRoutingInfoForSMInd\n%s\nmsisdn=%s", result.c_str(),getMSISDN());
    /* free decoded tree */
    def->free_struct(def, structure, 0);
  }
}
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
