/* ************************************************************************* *
 * BER Decoder: CHOICE type element decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_CHOICE_ELEMENT_DECODER
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_CHOICE_ELEMENT_DECODER

#include "eyeline/asn1/BER/rtdec/ElementDecoderByTag.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class CHCElementDecoderAC : public ElementDecoderByTagAC {
protected:
  // ----------------------------------------------------------
  // ElementDecoderAC Interface methods (protected)
  // ----------------------------------------------------------
  //Returns blocking mode for alternative with given UId
  uint8_t getBlocking(uint16_t alt_uid) const /*throw(std::exception)*/
  {
    return EDAlternative::blockALL;
  }

  //NOTE: copying constructor of successsor MUST properly set _alrArr
  CHCElementDecoderAC(const CHCElementDecoderAC & use_obj)
    : ElementDecoderByTagAC(use_obj)
  { }

public:
  CHCElementDecoderAC(EDAlternativesArray & alt_store)
    : ElementDecoderByTagAC(alt_store, ElementDecoderByTagAC::orderNone)
  { }
  ~CHCElementDecoderAC()
  { }
};


template <uint16_t _SizeTArg>
class CHCElementDecoder_T : public CHCElementDecoderAC {
private:
  eyeline::util::LWArray_T<EDAlternative, uint16_t, _SizeTArg> _altStore;

public:
  CHCElementDecoder_T()
    : CHCElementDecoderAC(_altStore)
  { }
  CHCElementDecoder_T(const CHCElementDecoder_T & use_obj)
    : CHCElementDecoderAC(use_obj)
  {
    setAltStorage(_altStore);
  }

  ~CHCElementDecoder_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_CHOICE_ELEMENT_DECODER */

