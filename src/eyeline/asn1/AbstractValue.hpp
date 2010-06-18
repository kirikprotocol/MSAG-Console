/* ************************************************************************* *
 * ASN.1 ABSTRACT-SYNTAX.&Type value implementation
 * ************************************************************************* */
#ifndef __ABSTRACT_VALUE_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_VALUE_DEFS__

#include "eyeline/asn1/AbstractSyntax.hpp"
#include "eyeline/util/MaxSizeof.hpp"

namespace eyeline {
namespace asn1 {

// Base abstract class for arbitrary value of single ASN.1 type from some defined
// ABSTRACT-SYNTAX. Value may converted to/from TransferSyntax representation
// (i.e encoded/decoded).
class ASTypeValueAC {
public:
  ASTypeValueAC()
  { }
  virtual ~ASTypeValueAC()
  { }

  //Returns true if given TransferSyntax is defined for this AbstractValue
  //virtual bool isTSdefined(TransferSyntax::Rule_e use_rule) const /*throw()*/ = 0;
  //Returns encoder that able to encode this value according to given TransferSyntax
  virtual ASTypeEncoderAC * getEncoder(TransferSyntax::Rule_e use_rule) /*throw(std::exception)*/ = 0;
  //Returns decoder that able to decode this value according to given TransferSyntax
  virtual ASTypeDecoderAC * getDecoder(TransferSyntax::Rule_e use_rule) /*throw(std::exception)*/ = 0;
};


template <class _TArg>
class ASTypeEncoderNotImplemented_T : public ASTypeEncoderOf_T<_TArg> {
public:
  class UnimplementedException : public smsc::util::Exception {
  public:
    UnimplementedException()
      : smsc::util::Exception("ASTypeEncoder: not implemented")
    { }
  };

  ASTypeEncoderNotImplemented(TransferSyntax::Rule_e use_rule = TransferSyntax::undefinedER)
    : ASTypeEncoderOf_T<_TArg>(TransferSyntax::undefinedER)
  { }
  ~ASTypeEncoderNotImplemented()
  { }

  void setValue(const _TArg & use_value) /*throw(std::exception)*/
  {
    throw UnimplementedException();
  }
  bool setTSRule(TransferSyntax::Rule_e use_rule) /*throw(std::exception)*/
  {
    throw UnimplementedException();
  }
  bool isTSsupported(TransferSyntax::Rule_e use_rule) const /*throw()*/
  {
    return false;
  }
  ENCResult calculate(void) /*throw(std::exception)*/
  {
    throw UnimplementedException();
  }
  //Composes the complete encoding of type value according to
  //TransferSyntax encoding rules. 
  ENCResult encode(uint8_t * use_enc, TSLength max_len) /*throw(std::exception)*/
  {
    throw UnimplementedException();
  }
};

template <class _TArg>
class ASTypeDecoderNotImplemented_T : public ASTypeDecoderOf_T<_TArg> {
public:
  UnimplementedException()
    : smsc::util::Exception("ASTypeDecoder: not implemented")
  { }

  ASTypeDecoderNotImplemented_T(TransferSyntax::Rule_e use_rule = TransferSyntax::undefinedER)
    : ASTypeDecoderOf_T<_TArg>(TransferSyntax::undefinedER)
  { }
  ~ASTypeDecoderNotImplemented_T()
  { }

  void setValue(_TArg & use_value) /*throw(std::exception)*/
  {
    throw UnimplementedException();
  }
  bool setTSRule(TransferSyntax::Rule_e use_rule) /*throw(std::exception)*/
  {
    throw UnimplementedException();
  }
  bool isTSsupported(TransferSyntax::Rule_e use_rule) const /*throw()*/
  {
    return false;
  }
  //Composes the complete encoding of type value according to
  //TransferSyntax encoding rules. 
  DECResult decode(const uint8_t * use_enc, TSLength max_len) /*throw(std::exception)*/
  {
    throw UnimplementedException();
  }
};


// Template for arbitrary value of single ASN.1 type from some defined
// ABSTRACT-SYNTAX. Value may converted to/from TransferSyntax representation
// (i.e encoded/decoded).
template <
    class _TArg
  , class EncoderOf_TArg /* : public ASTypeEncoderOf_T<_TArg> */
    = ASTypeEncoderNotImplemented_T<_TArg>
  , class DecoderOf_TArg /* : public ASTypeDecoderOf_T<_TArg> */
    = ASTypeDecoderNotImplemented_T<_TArg>
>
class ASTypeValue_T : public _TArg, public ASTypeValueAC {
private:
  enum Codec_e { cdcNone, cdcEncoder, cdcDecoder };

  union {
    void * aligner;
    uint8_t buf[eyeline::util::MaxSizeOf2_T<EncoderOf_TArg, DecoderOf_TArg>::VALUE];
  } _memCdc;

  union {
    EncoderOf_TArg * enc;
    DecoderOf_TArg * dec;
  } _pCdc;

  Codec_e _cdcType;

protected:
  EncoderOf_TArg * initOptEncoder(TransferSyntax::Rule_e use_rule)
  {
    _cdcType = cdcEncoder;
    _pCdc.enc = new (_memCdc.buf)EncoderOf_TArg(use_rule);
    _pCdc.enc->setValue(use_val);
    return _pCdc.enc;
  }
  //
  DecoderOf_TArg * initOptDecoder(TransferSyntax::Rule_e use_rule)
  {
    _cdcType = cdcDecoder;
    _pCdc.dec = new (_memCdc.buf)DecoderOf_TArg(use_rule);
    _pCdc.dec->setValue(use_val);
    return _pCdc.dec;
  }
  //
  EncoderOf_TArg * getOptEncoder(TransferSyntax::Rule_e use_rule)
  {
    if (_cdcType != cdcEncoder) {
      if (_cdcType == cdcDecoder)
        _pCdc.dec->~DecoderOf_TArg();
      return initOptEncoder(use_rule);
    }
    if (_pCdc.enc->getTSRule() != use_rule)
      _pCdc.enc->setTSRule(use_rule);
    return _pCdc.enc;
  }
  //
  DecoderOf_TArg * getOptDecoder(TransferSyntax::Rule_e use_rule)
  {
    if (_cdcType != cdcDecoder) {
      if (_cdcType == cdcEncoder)
        _pCdc.dec->~EncoderOf_TArg();
      return initOptDecoder(use_rule);
    }
    if (_pCdc.dec->getTSRule() != use_rule)
      _pCdc.dec->setTSRule(use_rule);
    return _pCdc.dec;
  }
  //
  void resetCodec(void)
  {
    if (_cdcType == cdcEncoder)
      _pCdc.enc->~EncoderOf_TArg();
    else if (_cdcType == cdcDecoder)
      _pCdc.dec->~DecoderOf_TArg();
    _pCdc.enc = 0;
    _cdcType == cdcNone;
  }

public:
  ASTypeValue_T() : ASTypeValueAC()
  {
    _pVal.enc = _memCdc.aligner = 0;
  }
  ~ASTypeValue_T()
  {
    resetCodec();
  }

  // -----------------------------------------------
  // -- ASTypeValueAC interface methods
  // -----------------------------------------------

  //Returns encoder that able to encode this value
  ASTypeEncoderAC * getEncoder(TransferSyntax::Rule_e use_rule) /*throw(std::exception)*/
  {
    return getOptEncoder(use_rule);
  }
  //Returns decoder that able to dencode this value
  ASTypeDecoderAC * getDecoder(TransferSyntax::Rule_e use_rule) /*throw(std::exception)*/
  {
    return getOptDecoder(use_rule);
  }
};

} //asn1
} //eyeline

#endif /* __ABSTRACT_VALUE_DEFS__ */

