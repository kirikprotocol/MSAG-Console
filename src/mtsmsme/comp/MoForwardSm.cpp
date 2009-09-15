static char const ident[] = "$Id$";

#include "mtsmsme/comp/MoForwardSm.hpp"
namespace smsc{namespace mtsmsme{namespace comp{

using std::vector;
using std::string;
using smsc::mtsmsme::processor::util::packNumString2BCD;
using smsc::mtsmsme::processor::util::packNumString2BCD91;
using smsc::mtsmsme::processor::util::unpackBCD912NumString;


MoForwardSmReq::MoForwardSmReq(const string& _smsca, const string& _msisdn, const vector<unsigned char>& _ui )
{
  arg.extensionContainer = 0;
  arg.imsi = 0;

  ZERO_OCTET_STRING(da);
  da.size = (int)packNumString2BCD91(da.buf, _smsca.c_str(), _smsca.length());
  arg.sm_RP_DA.present = SM_RP_DA_PR_serviceCentreAddressDA;
  arg.sm_RP_DA.choice.serviceCentreAddressDA.size = da.size;
  arg.sm_RP_DA.choice.serviceCentreAddressDA.buf = da.buf;

  ZERO_OCTET_STRING(oa);
  oa.size = (int)packNumString2BCD91(oa.buf, _msisdn.c_str(), _msisdn.length());
  arg.sm_RP_OA.present = SM_RP_OA_PR_msisdn;
  arg.sm_RP_OA.choice.msisdn.size = oa.size;
  arg.sm_RP_OA.choice.msisdn.buf = oa.buf;

  ZERO_OCTET_STRING(ui);
  ui.size = (int)_ui.size();
  memcpy(ui.buf,&_ui[0],_ui.size());
  arg.sm_RP_UI.size = ui.size;
  arg.sm_RP_UI.buf = ui.buf;
}
MoForwardSmReq::~MoForwardSmReq()
{
}
void MoForwardSmReq::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_MO_ForwardSM_Arg, &arg, print2vec, &buf);
}
void MoForwardSmReq::decode(const vector<unsigned char>& buf)
{
  return;
}
MoForwardSmInd::MoForwardSmInd(Logger* _logger) : logger(_logger) {}
void MoForwardSmInd::encode(vector<unsigned char>& buf) { return; }
void MoForwardSmInd::decode(const vector<unsigned char>& buf)
{
  void *structure = 0;
  asn_codec_ctx_t s_codec_ctx;
  asn_codec_ctx_t *opt_codec_ctx = 0;
  opt_codec_ctx = &s_codec_ctx;
  asn_dec_rval_t rval;
  asn_TYPE_descriptor_t *def = &asn_DEF_MO_ForwardSM_Arg;

  rval = ber_decode(0/*opt_codec_ctx*/, &asn_DEF_MO_ForwardSM_Arg,
      (void **) &structure, (void *)&buf[0], buf.size());

  smsc_log_debug(logger,
      "MoForwardSmInd::decode consumes %d/%d and returns code %d", rval.consumed,
      buf.size(), rval.code);
  if (structure)
  {
    /* Invoke type-specific printer */
    std::vector<unsigned char> stream;
    def->print_struct(def, structure, 1, print2vec, &stream);
    /* Create and return resulting string */
    string result((char*)&stream[0],(char*)&stream[0]+stream.size());
    smsc_log_debug(logger, "MoForwardSmInd\n%s", result.c_str());
    /* free decoded tree */
    def->free_struct(def, structure, 0);
  }
}
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
