/* ************************************************************************* *
 * BER Decoder: OCTET STRING type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_OCTSTR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_OCTSTR

#include "eyeline/asn1/OCTSTR.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeRCSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <
  typename _SizeTypeArg  //must be an unsigned integer type!
>
class OSTRDecConverter_T : public RCSDecConverterIface {
public:
  typedef typename eyeline::util::LWArrayExtension_T<uint8_t, _SizeTypeArg> ArrayType;

protected:
  _SizeTypeArg _maxSize;
  ArrayType *   _dVal;

public:
  explicit OSTRDecConverter_T(ArrayType * use_val = 0)
    : _maxSize(0), _dVal(use_val)
  { }
  ~OSTRDecConverter_T()
  { }

  void setValue(ArrayType & use_val, _SizeTypeArg max_sz = 0)
  {
    _dVal = &use_val; _maxSize = max_sz;
  }

  // -------------------------------------------
  // RCSDecConverterIface implementation
  // -------------------------------------------
  //Returns universal tag of fragment in case of fragmented string encoding
  const ASTag & fragmentTag(void) const /* throw()*/ { return _uniTag().OCTSTR; }
  //Incrementally converts next fragment of string encoding to
  //characters from associated alphabet.
  //Returns number of bytes consumed from buffer.
  DECResult unpackFragment(const uint8_t * use_buf, TSLength req_bytes) /* throw()*/
  {
    _SizeTypeArg numOcts = DOWNCAST_UNSIGNED(req_bytes, _SizeTypeArg);
    if (numOcts < req_bytes)
      return DECResult(DECResult::decBadVal); //too long value

    DECResult rval(DECResult::decOk);

    if (!_dVal->append(use_buf, numOcts)) {
      rval.status = DECResult::decBadVal; //max size exceeded
    } else {
      rval.nbytes = numOcts;
      if (_maxSize && (_dVal->size() > _maxSize))
        rval.status = DECResult::decBadVal;
    }
    return rval;
  }
};

/* ************************************************************************* *
 * Decodes by BER/DER/CER the OCTET STRING value encoding according to X.690
 * clause 8.7 (with appropriate DER/CER restrctions).
 * ************************************************************************* */
template <
  typename _SizeTypeArg  //must be an unsigned integer type!
>
class DecoderOfOCTSTR_T : public RCSTRValueDecoder {
protected:
  OSTRDecConverter_T<_SizeTypeArg> _vCvt;

public:
  typedef typename eyeline::util::LWArrayExtension_T<uint8_t, _SizeTypeArg> ArrayType;

  //Constructor for base type
  explicit DecoderOfOCTSTR_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : RCSTRValueDecoder(_vCvt, asn1::_uniTagging().OCTSTR, use_rule)
  { }
  //Constructor for tagged base type
  DecoderOfOCTSTR_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                   TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : RCSTRValueDecoder(_vCvt, ASTagging(use_tag, tag_env, asn1::_uniTagging().OCTSTR), use_rule)
  { }
  virtual ~DecoderOfOCTSTR_T()
  { }

  void setValue(ArrayType & use_val, _SizeTypeArg max_sz = 0)
  {
    _vCvt.setValue(use_val, max_sz);
  }
};

typedef DecoderOfOCTSTR_T<uint8_t>  DecoderOfOCTSTRTiny;
typedef DecoderOfOCTSTR_T<uint16_t> DecoderOfOCTSTR;

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_OCTSTR */

