/* ************************************************************************* *
 * BER Decoder: SEQUENCE type element decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_SEQUENCE_ELEMENT_DECODER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_SEQUENCE_ELEMENT_DECODER

#include "eyeline/asn1/BER/rtdec/ElementDecoderByDef.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <uint16_t _NumOfElemsArg, uint16_t _NumOfMandatoryArg>
class SEQElementDecoder_T : public ElementDecoderByTDef {
private:
  typedef eyeline::util::LWArray_T<EDAlternative, uint16_t, _NumOfElemsArg> EDAlternativesStore;
  typedef eyeline::util::LWArray_T<EDAOccurence, uint16_t, _NumOfElemsArg> EDAOccurenceStore;
  //NOTE: in order to avoid reallocation of TDMatrix, its capacity must be at
  //      least (number_of_mandatory_elements + 1)
  typedef eyeline::util::LWArray_T<TDMatrixRow, uint16_t, _NumOfMandatoryArg + 1> TDMatrixStore;
  typedef eyeline::util::BITArray_T<uint16_t, _NumOfElemsArg> EDAOccurencesBitStore;

  EDAlternativesStore   _altStore;
  EDAOccurenceStore     _occStore;
  TDMatrixStore         _tdMatrixStore;
  EDAOccurencesBitStore _occBitStore;

public:
  SEQElementDecoder_T()
    : ElementDecoderByTDef(_altStore, _occStore, _tdMatrixStore, _occBitStore)
  { }
  //
  SEQElementDecoder_T(const SEQElementDecoder_T & use_obj)
    : ElementDecoderByTDef(use_obj)
    , _altStore(use_obj._altStore), _occStore(use_obj._occStore)
    , _occBitStore(use_obj._occBitStore)
  {
    setStorages(_altStore, _occStore, _tdMatrixStore, _occBitStore);
  }
  //
  ~SEQElementDecoder_T()
  { }
};


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SET_ELEMENT_DECODER */

