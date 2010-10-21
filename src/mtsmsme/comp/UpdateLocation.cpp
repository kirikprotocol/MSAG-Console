static char const ident[] = "$Id$";

#include "mtsmsme/comp/UpdateLocation.hpp"
#include "mtsmsme/processor/util.hpp"
namespace smsc{namespace mtsmsme{namespace comp{

using smsc::mtsmsme::processor::net_loc_upd_v3;
using smsc::mtsmsme::processor::pduoid;
using smsc::mtsmsme::processor::tcapversion;
using smsc::mtsmsme::processor::util::packNumString2BCD;
using smsc::mtsmsme::processor::util::packNumString2BCD91;



using std::vector;
using std::string;

static uint8_t bits[] = {0x80};
BIT_STRING_t phases = {bits,sizeof(bits),0x05,};
void UpdateLocationReq::setParameters(const string& imsi, const string& msc, const string& vlr)
{
	arg.lmsi = 0;
	arg.extensionContainer = 0;
	arg.add_info = 0;
	arg.v_gmlc_Address = 0;
	arg.cs_LCS_NotSupportedByUE = 0;
	arg.informPreviousNetworkEntity = 0;

	ZERO_OCTET_STRING(_imsi);
	_imsi.size = (int)packNumString2BCD(_imsi.buf, imsi.c_str(), (unsigned)imsi.length());
	arg.imsi = _imsi;

	ZERO_OCTET_STRING(_msc);
	_msc.size = (int)packNumString2BCD91(_msc.buf, msc.c_str(), (unsigned)msc.length());
	arg.msc_Number = _msc;

	ZERO_OCTET_STRING(_vlr);
	_vlr.size = (int)packNumString2BCD91(_vlr.buf, vlr.c_str(), (unsigned)vlr.length());
	arg.vlr_Number = _vlr;

	//vlrcap.supportedCamelPhases = &phases;
	vlrcap.supportedCamelPhases = 0;
	vlrcap.extensionContainer = 0;
	vlrcap.solsaSupportIndicator = 0;
	vlrcap.istSupportIndicator = 0;
	vlrcap.superChargerSupportedInServingNetworkEntity = 0;
	vlrcap.longFTN_Supported = 0;
	vlrcap.supportedLCS_CapabilitySets = 0;
	vlrcap.offeredCamel4CSIs = 0;
	arg.vlr_Capability = &vlrcap;
}
void UpdateLocationReq::encode(vector<unsigned char>& buf)
{
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_UpdateLocationArg, &arg, print2vec, &buf);
}
void UpdateLocationReq::decode(const vector<unsigned char>& buf)
{
  return;
}
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
