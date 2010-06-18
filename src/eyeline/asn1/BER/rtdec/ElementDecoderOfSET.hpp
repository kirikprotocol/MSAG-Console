/* ************************************************************************* *
 * BER Decoder: SET type element decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_SET_ELEMENT_DECODER
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_SET_ELEMENT_DECODER

#include "eyeline/asn1/BER/rtdec/ElementDecoderByTag.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class SETElementDecoderAC : public ElementDecoderByTagAC {
protected:
  // ----------------------------------------------------------
  // ElementDecoderAC Interface methods (protected)
  // ----------------------------------------------------------
  //Returns blocking mode for alternative with given UId
  uint8_t getBlocking(uint16_t alt_uid) const /*throw(std::exception)*/
  {
    if (_altsArr->at(alt_uid).isUnkExtension())
      return (_order == orderNone) ? EDAlternative::blockNone : EDAlternative::blockPreceeding;

    return (_order == orderNone) ? EDAlternative::blockItself : 
                                  EDAlternative::blockItself | EDAlternative::blockPreceeding;
  }

  //NOTE: copying constructor of successsor MUST properly set _alrArr
  SETElementDecoderAC(const SETElementDecoderAC & use_obj)
    : ElementDecoderByTagAC(use_obj)
  { }

public:
  SETElementDecoderAC(EDAlternativesSTORE & alt_store)
    : ElementDecoderByTagAC(alt_store, orderNone)
  { }
  ~SETElementDecoderAC()
  { }
};


template <uint16_t _SizeTArg>
class SETElementDecoder_T : public SETElementDecoderAC {
private:
  eyeline::util::LWArray_T<EDAlternative, uint16_t, _SizeTArg> _altStore;

public:
  SETElementDecoder_T(ElementDecoderByTagAC::Order_e use_order 
                                  = ElementDecoderByTagAC::orderNone)
    : SETElementDecoderAC(_altStore, use_order)
  { }
  SETElementDecoder_T(const SETElementDecoderAC & use_obj)
    : SETElementDecoderAC(use_obj)
  {
    setAltStorage(_altStore);
  }
  //
  ~SETElementDecoder_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SET_ELEMENT_DECODER */

