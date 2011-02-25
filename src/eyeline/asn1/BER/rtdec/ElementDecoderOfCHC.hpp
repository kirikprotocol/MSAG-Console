/* ************************************************************************* *
 * BER Decoder: CHOICE type element decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_CHOICE_ELEMENT_DECODER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_CHOICE_ELEMENT_DECODER

#include "eyeline/asn1/BER/rtdec/ElementDecoderByTag.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <uint16_t _SizeTArg>
class CHCElementDecoder_T : public ElementDecoderByTagAC {
private:
  eyeline::util::LWArray_T<EDAlternative, uint16_t, _SizeTArg> _altStore;

protected:
  // ----------------------------------------------------------
  // ElementDecoderAC Interface methods (protected)
  // ----------------------------------------------------------
  //Returns blocking mode for alternative with given UId
  virtual uint8_t getBlocking(uint16_t alt_uid) const /*throw(std::exception)*/
  {
    return EDAlternative::blockALL;
  }

public:
  CHCElementDecoder_T()
    : ElementDecoderByTagAC(_altStore, ElementDecoderByTagAC::orderNone)
  { }
  explicit CHCElementDecoder_T(const CHCElementDecoder_T & use_obj)
    : ElementDecoderByTagAC(use_obj)
  {
    setAltStorage(_altStore);
  }
  //
  ~CHCElementDecoder_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_CHOICE_ELEMENT_DECODER */

