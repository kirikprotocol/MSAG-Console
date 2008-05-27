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

UpdateLocationMessage::UpdateLocationMessage()
{
  begin.present = TCMessage_PR_begin;
  begin.choice.begin.dialoguePortion = 0;
  begin.choice.begin.components = 0;
  //initialize empty application context
  dp.encoding.choice.single_ASN1_type.choice.dialogueRequest.application_context_name.size=0;
  dp.encoding.choice.single_ASN1_type.choice.dialogueRequest.application_context_name.buf=0;
  // initialize application context
  ac = net_loc_upd_v3;
  dp.direct_reference = &pduoid;
  dp.indirect_reference = 0;
  dp.data_value_descriptor = 0;
  dp.encoding.present = encoding_PR_single_ASN1_type;
  dp.encoding.choice.single_ASN1_type.present = DialoguePDU_PR_dialogueRequest;
  AARQ_apdu_t& r = dp.encoding.choice.single_ASN1_type.choice.dialogueRequest;
  r.protocol_version = &tcapversion;
  OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&ac.arcs[0],sizeof(unsigned long),ac.arcs.size());
  r.aarq_user_information = 0;
  begin.choice.begin.dialoguePortion = ( struct EXT *)&dp;
}
UpdateLocationMessage::~UpdateLocationMessage()
{
  if(dp.encoding.choice.single_ASN1_type.choice.dialogueRequest.application_context_name.buf)
    free(dp.encoding.choice.single_ASN1_type.choice.dialogueRequest.application_context_name.buf);
}
void UpdateLocationMessage::setOTID(TrId _otid)
{
  memcpy(otid,_otid.buf,_otid.size);
  begin.choice.begin.otid.buf = otid;
  begin.choice.begin.otid.size = _otid.size;
}
static uint8_t bits[] = {0x80};
BIT_STRING_t phases = {bits,sizeof(bits),0x05,};
void UpdateLocationMessage::setComponent(const string& imsi, const string& msc, const string& vlr)
{

  // intialize invoke with argument
  arg.lmsi = 0;
  arg.extensionContainer = 0;
  arg.add_info = 0;
  arg.v_gmlc_Address = 0;
  arg.cs_LCS_NotSupportedByUE = 0;
  arg.informPreviousNetworkEntity = 0;

  ZERO_OCTET_STRING(_imsi);
  _imsi.size = packNumString2BCD(_imsi.buf, imsi.c_str(), imsi.length());
  arg.imsi = _imsi;

  ZERO_OCTET_STRING(_msc);
  _msc.size = packNumString2BCD91(_msc.buf, msc.c_str(), msc.length());
  arg.msc_Number = _msc;

  ZERO_OCTET_STRING(_vlr);
  _vlr.size = packNumString2BCD91(_vlr.buf, vlr.c_str(), vlr.length());
  arg.vlr_Number = _vlr;

  vlrcap.supportedCamelPhases = &phases;
  vlrcap.extensionContainer = 0;
  vlrcap.solsaSupportIndicator = 0;
  vlrcap.istSupportIndicator = 0;
  vlrcap.superChargerSupportedInServingNetworkEntity = 0;
  vlrcap.longFTN_Supported = 0;
  vlrcap.supportedLCS_CapabilitySets = 0;
  vlrcap.offeredCamel4CSIs = 0;
  arg.vlr_Capability = &vlrcap;

  // intialize invoke with invokeID and operation code
  comp.present = Component_PR_invoke;
  comp.choice.invoke.invokeId = 1;
  comp.choice.invoke.linkedId = 0;
  comp.choice.invoke.opcode.present = Code_PR_local;
  comp.choice.invoke.opcode.choice.local = 2; /* updateLocation operation */

  arr[0]= &comp;
  comps.list.count = 1;
  comps.list.size = 1;
  comps.list.array = arr;
  begin.choice.begin.components = &comps;
}
void UpdateLocationMessage::encode(vector<unsigned char>& buf)
{
  vector<unsigned char> arg_buf;
  ANY_t argument;
  asn_enc_rval_t er;
  er = der_encode(&asn_DEF_UpdateLocationArg, &arg, print2vec, &arg_buf);
  argument.size = arg_buf.size();
  argument.buf = &arg_buf[0];
  comp.choice.invoke.argument = &argument;
  er = der_encode(&asn_DEF_TCMessage, &begin, print2vec, &buf);
}
void UpdateLocationMessage::decode(const vector<unsigned char>& buf)
{
  return;
}
void UpdateLocationReq::setParameters(const string& imsi, const string& msc, const string& vlr)
{
	arg.lmsi = 0;
	arg.extensionContainer = 0;
	arg.add_info = 0;
	arg.v_gmlc_Address = 0;
	arg.cs_LCS_NotSupportedByUE = 0;
	arg.informPreviousNetworkEntity = 0;

	ZERO_OCTET_STRING(_imsi);
	_imsi.size = packNumString2BCD(_imsi.buf, imsi.c_str(), imsi.length());
	arg.imsi = _imsi;

	ZERO_OCTET_STRING(_msc);
	_msc.size = packNumString2BCD91(_msc.buf, msc.c_str(), msc.length());
	arg.msc_Number = _msc;

	ZERO_OCTET_STRING(_vlr);
	_vlr.size = packNumString2BCD91(_vlr.buf, vlr.c_str(), vlr.length());
	arg.vlr_Number = _vlr;

	vlrcap.supportedCamelPhases = &phases;
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
