static char const ident[] = "$Id$";

#include "mtsmsme/comp/ProvideRoamingNumber.hpp"
#include "mtsmsme/processor/util.hpp"
namespace smsc{namespace mtsmsme{namespace comp{

using std::vector;
using std::string;
using smsc::mtsmsme::processor::util::packNumString2BCD;
using smsc::mtsmsme::processor::util::packNumString2BCD91;

void ProvideRoamingNumberResp::setRoamingNumber(const string& number)
{
    arg.extensionContainer = 0;
    arg.releaseResourcesSupported = 0;
    ZERO_OCTET_STRING(_number);
    _number.size = packNumString2BCD91(_number.buf, number.c_str(), number.length());
    arg.roamingNumber = _number;
}
void ProvideRoamingNumberResp::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_ProvideRoamingNumberRes, &arg, print2vec, &buf);
}
void ProvideRoamingNumberResp::decode(const vector<unsigned char>& buf)
{
  return;
}
void ProvideRoamingNumberRespV1::setRoamingNumber(const string& number)
{
    ZERO_OCTET_STRING(_number);
    _number.size = packNumString2BCD91(_number.buf, number.c_str(), number.length());
}
void ProvideRoamingNumberRespV1::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_OCTET_STRING, &_number, print2vec, &buf);
}
void ProvideRoamingNumberRespV1::decode(const vector<unsigned char>& buf)
{
  return;
}
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
