/* ************************************************************************* *
 * BER Decoder: Base class of all structured type decoders.
 *              (SET/SEQUENCE, SET OF/SEQUENCE OF)
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_STRUCT
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_STRUCT

#include "eyeline/asn1/BER/rtdec/ElementDecoder.hpp"
#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class DecoderOfStructAC : public TypeValueDecoderAC {
private:
  ElementDecoderAC * _elDec; //NOTE: actually it's just a reference to a successor
                             //member, so its copying constructor MUST properly set
                             //that pointer.
protected:
  DECResult decodeElement(const uint8_t * use_enc, TSLength max_len,
                          bool relaxed_rule) /*throw(std::exception)*/;

  // -- ************************************************* --
  // -- ValueDecoderIface abstract methods are to implement
  // -- ************************************************* --
  virtual DECResult decodeVAL(const TLVProperty * val_prop,
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false)
    /*throw(std::esception)*/;

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */ = 0;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ = 0;


  void setElementDecoder(ElementDecoderAC & elm_dec) { _elDec = &elm_dec; }

  //NOTE: copying constructor of successsor MUST properly set _elDec by
  //      calling setElementDecoder()
  DecoderOfStructAC(const DecoderOfStructAC & use_obj)
    : TypeValueDecoderAC(use_obj), _elDec(0)
  { }

public:
  DecoderOfStructAC(ElementDecoderAC & use_eldec, const ASTagging & eff_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(eff_tags, use_rule), _elDec(&use_eldec)
  { }
  virtual ~DecoderOfStructAC()
  { }

  //adds tagged field
  void setField(uint16_t unique_idx, const ASTag & fld_tag,
                ASTagging::Environment_e fld_env, EDAlternative::Presence_e use_presence)
    /*throw(std::exception)*/
  {
    _elDec->setAlternative(unique_idx, fld_tag, fld_env, use_presence);
  }
  //adds an untagged field of ordinary type
  void setField(uint16_t unique_idx, const ASTag & use_tag,
                      EDAlternative::Presence_e use_presence)
    /*throw(std::exception)*/
  {
    _elDec->setAlternative(unique_idx, use_tag, use_presence);
  }
  //adds an untagged field of ANY/Opentype
  void setField(uint16_t unique_idx, EDAlternative::Presence_e use_presence)
        /*throw(std::exception)*/
  {
    _elDec->setAlternative(unique_idx, use_presence);
  }
  //adds an untagged field of untagged CHOICE type
  void setField(uint16_t unique_idx, const TaggingOptions & use_tag_opts,
                      EDAlternative::Presence_e use_presence)
    /*throw(std::exception)*/
  {
    _elDec->setAlternative(unique_idx, use_tag_opts, use_presence);
  }
  //adds unknown extension additions entry
  void setUnkExtension(uint16_t unique_idx) /*throw(std::exception)*/
  {
    _elDec->setUnkExtension(unique_idx);
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_STRUCT */

