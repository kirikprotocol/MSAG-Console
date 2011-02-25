/* ************************************************************************* *
 * BER Decoder: Base class of all structured type decoders.
 *              (SET/SEQUENCE, SET OF/SEQUENCE OF)
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_STRUCT
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_STRUCT

#include "eyeline/asn1/BER/rtdec/DecodeConstructed.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class DecoderOfStructAC : public DecoderOfConstructedAC {
protected:
  // ------------------------------------------------
  // -- ValueDecoderIface interface methods
  // ------------------------------------------------
  //NOTE: should reset ElementDecoder prior to decoding!
  virtual DECResult decodeVAL(const TLParser & tlv_prop, //corresponds to innermost identification tag
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false) /*throw(std::esception)*/;

  //NOTE: copying constructor of successsor MUST properly set _elDec by
  //      calling setElementDecoder()
  explicit DecoderOfStructAC(const DecoderOfStructAC & use_obj)
    : DecoderOfConstructedAC(use_obj)
  { }

  DecoderOfStructAC(ElementDecoderAC & use_eldec, const ASTagging & eff_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfConstructedAC(use_eldec, eff_tags, use_rule)
  { }

  // ----------------------------------------------------------
  // Initialization methods for successors
  // ----------------------------------------------------------

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

public:
  virtual ~DecoderOfStructAC()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_STRUCT */

