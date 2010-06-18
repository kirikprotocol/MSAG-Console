/* ************************************************************************* *
 * BER Encoder: ENUMERATED type encoder
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_ENUM
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_ENUM

#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class EncoderOfENUM : public EncoderOfINTEGER {
protected:

  //Tagged type referencing INTEGER. 
  // NOTE: eff_tags is a complete effective tagging of type!
  EncoderOfENUM(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : EncoderOfINTEGER(eff_tags, use_rule)
  { }

public:
  EncoderOfENUM(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : EncoderOfINTEGER(asn1::_tagsENUM, use_rule)
  { }
  //Tagged ENUMERATED type encoder constructor
  EncoderOfENUM(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : EncoderOfINTEGER(ASTagging(use_tag, tag_env, asn1::_tagsENUM), use_rule)
  { }
  ~EncoderOfENUM()
  { }
};

};
} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_ENUM */

