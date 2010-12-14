/* ************************************************************************* *
 * BER Decoder: 'TLV' encoding decoder definitions.
 * ************************************************************************* */
#ifndef __ASN1_BER_TAGGING_DECODER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_TAGGING_DECODER

#include "eyeline/asn1/BER/rtutl/TLVUtils.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::TSLength;
using eyeline::asn1::DECResult;

/*
class TLVParserException : public smsc::util::Exception {
protected:
  DECResult _dRes;

public:
  TLVParserException(const DECResult & dec_res, const char* const fmt, ...)
    : _dRes(dec_res)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }

  const DECResult & getResult(void) const throw() { return _dRes; }
};
*/

class TaggingDecoder {
private:
  eyeline::util::LWArray_T<TLParser, uint8_t, _ASTaggingDFLT_SZ> _tlws;
  const ASTagging * _effTags; //original complete tagging of type
  const TLParser *  _outerTL; //either outermost tag or implicit tag

public:
  //Empty constructor: for later initialization
  explicit TaggingDecoder(const ASTagging * use_tags = NULL,
                          const TLParser * outer_tl = NULL)
    : _tlws(use_tags ? use_tags->size() : 0)
    , _effTags(use_tags), _outerTL(outer_tl)
  { }

  void init(const ASTagging * use_tags, const TLParser * outer_tl = NULL)
  {
    _effTags = use_tags;
    _outerTL = outer_tl;
    _tlws.clear(); 
    _tlws.reserve(_effTags->size());
  }

  //Decodes all TL-pairs of TLV layout
  DECResult decodeBOC(const uint8_t * use_enc, TSLength max_len,
                      bool relaxed_rule = false) /*throw(std::exception)*/;

  //Decodes all 'end-of-content' octets of TLV layout
  DECResult decodeEOC(const uint8_t * use_enc, TSLength max_len) const /*throw()*/;

  //Returns 'V'-part encoding properties.
  //NOTE: decodeBOC() or setOutermostTag() MUST be called prior to this call.
  const TLVProperty * getVProperties(void) const /*throw()*/
  {
    return (_outerTL && (_tlws.size() < 2)) ? 
            _outerTL : (_tlws.empty() ? NULL : &_tlws.atLast());
  } 
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TAGGING_DECODER */

