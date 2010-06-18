/* ************************************************************************* *
 * BER Decoder: BIT STRING type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_BITSTR
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_BITSTR

#include "eyeline/asn1/BITSTR.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeRCSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <
  typename _SizeTypeArg  //must be an unsigned integer type!
>
class BSTRDecConverter_T : public RCSDecConverterIface {
public:
  typedef typename eyeline::util::BITArrayExtension_T<_SizeTypeArg> ArrayType;

protected:
  uint8_t     _prvUnused;
  ArrayType * _dVal;

public:
  BSTRDecConverter_T(ArrayType * use_val = 0)
    : _prvUnused(0), _dVal(use_val)
  { }
  ~BSTRDecConverter_T()
  { }

  void setValue(ArrayType & use_val) { _dVal = &use_val; }

  // -------------------------------------------
  // RCSDecConverterIface implementation
  // -------------------------------------------
  //Returns universal tag of fragment in case of fragmented string encoding
  const ASTag & fragmentTag(void) const /* throw()*/ { return _tagBITSTR; }
  //Incrementally converts next fragment of string encoding to
  //characters from associated alphabet.
  //Returns number of bytes consumed from buffer.
  DECResult unpackFragment(const uint8_t * use_buf, TSLength req_bytes) /* throw()*/
  {
    _SizeTypeArg numOcts = DOWNCAST_UNSIGNED(req_bytes, _SizeTypeArg);
    DECResult rval(DECResult::decOk);

    //NOTE: intermediate fragments MUST BE octet aligned
    if ((numOcts < 2) || _prvUnused) {
      rval.status = DECResult::decBadEncoding;
      return rval;
    }
    _prvUnused = *use_buf & 0x07;
    ++rval.nbytes;
    if (_dVal->append(use_buf + 1, (numOcts<<3) - _prvUnused)) {
      rval.nbytes += numOcts;
      if (rval.nbytes < req_bytes)
        rval.status = DECResult::decBadVal;
    } else //max size exceeded
      rval.status = DECResult::decBadVal;
    return rval;
  }
};

/* ************************************************************************* *
 * Decodes by BER/DER/CER the BIT STRING value encoding according to X.690
 * clause 8.6 (with appropriate DER/CER restrctions).
 * ************************************************************************* */
template <
  typename _SizeTypeArg  //must be an unsigned integer type!
>
class DecoderOfBITSTR_T : public RCSTRValueDecoder {
protected:
  BSTRDecConverter_T<_SizeTypeArg> _vCvt;

public:
  typedef typename eyeline::util::BITArrayExtension_T<_SizeTypeArg> ArrayType;

  //Constructor for base type
  DecoderOfBITSTR_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : RCSTRValueDecoder(_vCvt, asn1::_tagsBITSTR, use_rule)
  { }
  //Constructor for tagged base type
  DecoderOfBITSTR_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                   TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : RCSTRValueDecoder(_vCvt, ASTagging(use_tag, tag_env, asn1::_tagsBITSTR), use_rule)
  { }
  virtual ~DecoderOfBITSTR_T()
  { }

  void setValue(ArrayType & use_val) { _vCvt.setValue(use_val); }
};

typedef DecoderOfBITSTR_T<uint8_t>  DecoderOfBITSTRTiny;
typedef DecoderOfBITSTR_T<uint16_t> DecoderOfBITSTR;

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_BITSTR */

