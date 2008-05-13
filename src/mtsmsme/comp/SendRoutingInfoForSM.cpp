static char const ident[] = "$Id$";

#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "mtsmsme/processor/util.hpp"
namespace smsc{namespace mtsmsme{namespace comp{

using std::vector;
using std::string;
using smsc::mtsmsme::processor::util::packNumString2BCD;
using smsc::mtsmsme::processor::util::packNumString2BCD91;

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

/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }