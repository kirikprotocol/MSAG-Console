#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/enc/TEDialoguePortion.hpp"


namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TEDialoguePortionStructured::_typeTags(asn1::ASTag::tagApplication,
                      11, asn1::ASTagging::tagsEXPLICIT);


//Creates and initializes AARQ_APdu
TEAPduAARQ *
  TEDialoguePortionStructured::initPduAARQ(const asn1::EncodedOID & use_app_ctx) /*throw(std::exception)*/
{
  _pduEnc.aarq().init(getTSRule()).setAppContext(use_app_ctx);
  asn1::ber::EncoderOfExternal::setValue(_ac_tcap_strDialogue_as, *_pduEnc.aarq().get());
  return _pduEnc.aarq().get();
}

//Creates and initializes AARE_APdu
TEAPduAARE *
  TEDialoguePortionStructured::initPduAARE(const asn1::EncodedOID & use_app_ctx) /*throw(std::exception)*/
{
  _pduEnc.aare().init(getTSRule()).setAppContext(use_app_ctx);
  asn1::ber::EncoderOfExternal::setValue(_ac_tcap_strDialogue_as, *_pduEnc.aare().get());
  return _pduEnc.aare().get();
}

//Creates and initializes ABRT_APdu
TEAPduABRT *
  TEDialoguePortionStructured::initPduABRT(TDialogueAssociate::AbrtSource_e abrt_src) /*throw(std::exception)*/
{
  _pduEnc.abrt().init(getTSRule()).setAbortSource(abrt_src);
  asn1::ber::EncoderOfExternal::setValue(_ac_tcap_strDialogue_as, *_pduEnc.abrt().get());
  return _pduEnc.abrt().get();
}

}}}}

