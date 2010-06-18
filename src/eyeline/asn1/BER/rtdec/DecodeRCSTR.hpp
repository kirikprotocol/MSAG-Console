/* ************************************************************************* *
 * BER Decoder: Generic Restricted Character String decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_RCSTR
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_RCSTR

#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes value of Restricted Character String type from its encoding
 * (possibly fragmented) performing necessary character transformations
 * (escaping, etc).
 * ************************************************************************* */
class RCSDecConverterIface {
protected:
  virtual ~RCSDecConverterIface();

public:
  //Returns universal tag of fragment in case of fragmented string encoding
  virtual const ASTag & fragmentTag(void) const /* throw()*/ = 0;
  //Incrementally converts next fragment of string encoding to
  //characters from associated alphabet.
  //Returns number of bytes consumed from buffer.
  virtual DECResult unpackFragment(const uint8_t * use_buf, TSLength req_bytes) /* throw()*/= 0;
};


class RCSTRValueDecoder : public TypeValueDecoderAC {
protected:
  RCSDecConverterIface *  _strConv;

  // -----------------------------------------------------------
  // -- ValueDecoderIface interface methods
  // -----------------------------------------------------------
  DECResult decodeVAL(const TLVProperty * val_prop,
                      const uint8_t * use_enc, TSLength max_len,
                      TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                      bool relaxed_rule = false)
    /*throw(std::exception)*/;

public:
  // NOTE: eff_tags is a complete effective tagging of type!
  explicit RCSTRValueDecoder(RCSDecConverterIface & use_conv,
                             const ASTagging & eff_tags,
                             TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(eff_tags, use_rule), _strConv(&use_conv)
  { }
  ~RCSTRValueDecoder()
  { }
};


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_RCSTR */

