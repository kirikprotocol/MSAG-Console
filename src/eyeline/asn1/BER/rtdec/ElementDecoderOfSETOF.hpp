/* ************************************************************************* *
 * BER Decoder: SET OF type element decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_SET_OF_ELEMENT_DECODER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_SET_OF_ELEMENT_DECODER

#include "eyeline/asn1/BER/rtdec/ElementDecoderByTag.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class SETOFElementDecoder : public ElementDecoderByTagAC {
private:
  eyeline::util::LWArray_T<EDAlternative, uint16_t, 1> _altStore;

  //SET OF cann't has unknown extension additions !!!
  using ElementDecoderByTagAC::setUnkExtension;

protected:
  // ----------------------------------------------------------
  // ElementDecoderAC Interface methods (protected)
  // ----------------------------------------------------------
  //Returns blocking mode for alternative with given UId
  uint8_t getBlocking(uint16_t alt_uid) const /*throw(std::exception)*/
  {
    return EDAlternative::blockNone;
  }

public:
  SETOFElementDecoder()
    : ElementDecoderByTagAC(_altStore, ElementDecoderByTagAC::orderNone)
  { }
  SETOFElementDecoder(const SETOFElementDecoder & use_obj)
    : ElementDecoderByTagAC(use_obj)
    , _altStore(use_obj._altStore)
  {
    setAltStorage(_altStore);
  }
  //
  ~SETOFElementDecoder()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SET_OF_ELEMENT_DECODER */

