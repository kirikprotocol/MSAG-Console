/* ************************************************************************* *
 * BER Decoder: base class of all constructed (structured & sequenced) type 
 *              decoders.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_CONSTRUCTED
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_CONSTRUCTED

#include "eyeline/asn1/BER/rtdec/ElementDecoder.hpp"
#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
//
//Parses constructed type value encoding and decodes all containing elements
//according to specified ElementDecoder.
//
class DecoderOfConstructedAC : public TypeValueDecoderAC {
protected:
  ElementDecoderAC * _elDec; //NOTE: actually it's just a reference to a successor
                             //member, so its copying constructor MUST properly set
                             //that pointer.

  DECResult decodeElement(const TLParser & tlv_prop, //outermost element tag
                          const uint8_t * use_enc, TSLength max_len,
                          bool relaxed_rule) /*throw(std::exception)*/;

  // --------------------------------------------
  // -- ValueDecoderIface abstract methods 
  // --------------------------------------------
  //NOTE: should reset ElementDecoder prior to decoding!
  virtual DECResult decodeVAL(const TLParser & tlv_prop, //corresponds to innermost identification tag
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false)
    /*throw(std::esception)*/ = 0;

  // ---------------------------------------------
  // -- DecoderOfConstructedAC interface methods
  // ---------------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */ = 0;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */  { return; }


  void setElementDecoder(ElementDecoderAC & elm_dec) { _elDec = &elm_dec; }

  //NOTE: copying constructor of successsor MUST properly set _elDec 
  //      via setElementDecoder() call !!!
  explicit DecoderOfConstructedAC(const DecoderOfConstructedAC & use_obj)
    : TypeValueDecoderAC(use_obj), _elDec(0)
  { }

  DecoderOfConstructedAC(ElementDecoderAC & use_eldec,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(use_rule), _elDec(&use_eldec)
  { }

  DecoderOfConstructedAC(ElementDecoderAC & use_eldec, const ASTagging & eff_tags,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(eff_tags, use_rule), _elDec(&use_eldec)
  { }

public:
  virtual ~DecoderOfConstructedAC()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_CONSTRUCTED */

