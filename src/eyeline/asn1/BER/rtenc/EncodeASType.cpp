#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
#include "eyeline/asn1/BER/rtutl/TLVUtils.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {


/* ************************************************************************* *
 * Class EncoderOfAStype implementation:
 * ************************************************************************* */
void EncoderOfASType::setSelection(const ASTagging & use_tags, ValueEncoderIface * val_enc)
{
  if (!val_enc)
    throw smsc::util::Exception("ber::EncoderOfASType::setValue(): ValueEncoder isn't defined");

  _altTags.setTagging(use_tags);
  _tlvEnc.init(*val_enc, refreshTagging());   //compose complete tagging of that Opentype  
}

void EncoderOfASType::setValue(TypeEncoderAC & type_enc) /*throw(std::exception)*/
{
  if (!type_enc.getTagging())
    throw smsc::util::Exception("ber::EncoderOfASType::setValue(): tagging isn't defined");
  //use ValueEncoder of referenced type instead of own
  setSelection(*type_enc.getTagging(), type_enc.getVALEncoder());
}

void EncoderOfASType::setValue(const TransferSyntax & use_ts) /*throw(std::exception)*/
{
  if (!TSGroupBER::isPortable(TSGroupBER::getBERRule(getTSRule()), TSGroupBER::getBERRule(use_ts._rule)))
    throw smsc::util::Exception("ber::EncoderOfASType::setValue(): "
                                "TransferSyntax %s doesn't match the %s",
                                use_ts.nmRule(), TransferSyntax::nmRule(getTSRule()));

  //Split given TS into two part: outermost tag and rest of TLV encoding, than
  //treat outermost tag as tagging of CHOICE alternative, and the rest of TLV
  //encoding as an encoding of CHOICE alternative.
  TLParser  tlp;
  DECResult res = tlp.decodeBOC(use_ts.getPtr(), use_ts._maxlen);

  if (res.isOkRelaxed() && tlp.isIndefinite()) { //check EOCs
    if (!(use_ts.getPtr()[use_ts._maxlen-1] || use_ts.getPtr()[use_ts._maxlen-2]))
      res.status = DECResult::decBadEncoding;
  }
  if (!res.isOkRelaxed())
    throw smsc::util::Exception("ber::EncoderOfASType::setValue(): "
                                "TransferSyntax isn't a valid TLV");

  _vProp._ldForm = tlp._ldForm;
  _vProp._isConstructed = tlp._isConstructed;
  _vProp._valLen = use_ts._maxlen - tlp.getBOCsize();
  if (tlp.isIndefinite())
    _vProp._valLen -= 2;
  _valTS = use_ts.getPtr() + tlp.getBOCsize();

  //use own ValueEncoderIface
  setSelection(ASTagging(tlp._tag, ASTagging::tagsIMPLICIT), (ValueEncoderIface*)this);
}


// -- -------------------------------------- --
// -- ValueEncoderIface interface methods
// -- -------------------------------------- --

ENCResult EncoderOfASType::encodeVAL(uint8_t * use_enc, TSLength max_len) const
  /*throw(std::exception)*/
{
  if (max_len < _vProp._valLen)
    return ENCResult(ENCResult::encMoreMem);

  memcpy(use_enc, _valTS, _vProp._valLen);
  return ENCResult(ENCResult::encOk, _vProp._valLen);
}


} //ber
} //asn1
} //eyeline

