/* ************************************************************************* *
 * Class(es) implementing ASN.1 ABSTRACT-SYNTAX class
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ABSTRACT_SYNTAX_DEFS__

#include "eyeline/asn1/TransferSyntax.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace asn1 {

//Basic abstract class for encoder of ASN.1 defined type.
//NOTE: Encoder is created for particular transfer syntax rule but is allowed
//to support several ones.
class ASTypeEncoderAC {
protected:
  TransferSyntax::Rule_e _tsRule; //TransferSyntax encoding rules id

public:
  ASTypeEncoderAC(TransferSyntax::Rule_e use_rule)
    : _tsRule(use_rule)
  { }
  virtual ~ASTypeEncoderAC()
  { }

  //Returns currently assigned transferSyntax
  TransferSyntax::Rule_e getTSRule(void) const { return _tsRule; }
  //Sets required transferSyntax encoding rule.
  //Returns: true if encoding optimization data should be (re)calculated.
  //Throws: in case of unsupported rule.
  virtual bool setTSRule(TransferSyntax::Rule_e use_rule) /*throw(std::exception)*/
  {
    if (_tsRule == use_rule)
      return false;

    if (!isTSsupported(use_rule))
      throw smsc::util::Exception("asn1::ASTypeEncoderAC::setRule(): %s isn't supported",
                                 TransferSyntax::nmRule(use_rule));
    _tsRule = use_rule;
    return true;
  }

  // -- ------------------------------------------------- --
  // -- ASTypeEncoderAC abstract methods
  // -- ------------------------------------------------- --
  //Returns true if encoder able to compose encoding of requested TransferSyntax
  virtual bool isTSsupported(TransferSyntax::Rule_e use_rule) const /*throw()*/ = 0;
  //Calculates length of complete encoding without its composition.
  //NOTE: if calculation is impossible (f.ex. stream encoding) 
  //      ENCResult::encUnsupported is returned.
  virtual ENCResult calculate(void) /*throw(std::exception)*/ = 0;
  //Composes the complete encoding of type value according to
  //TransferSyntax encoding rules. 
  virtual ENCResult encode(uint8_t * use_enc, TSLength max_len) /*throw(std::exception)*/ = 0;
};

//
//template <class _TArg>
//class ASTypeEncoderOf_T : public ASTypeEncoderAC {
//public:
//  ASTypeEncoderOf_T(TransferSyntax::Rule_e use_rule)
//    : ASTypeEncoderAC(use_rule)
//  { }
//  virtual ~ASTypeEncoderOf_T()
//  { }
//
//  virtual void setValue(const _TArg & use_value) /*throw(std::exception)*/ = 0;
//};

//Basic abstract class for decoder of ASN.1 defined type.
class ASTypeDecoderAC {
protected:
  TransferSyntax::Rule_e _tsRule; //TransferSyntax encoding rules id

public:
  ASTypeDecoderAC(TransferSyntax::Rule_e use_rule)
    : _tsRule(use_rule)
  { }
  virtual ~ASTypeDecoderAC()
  { }

  //Returns currently assigned transferSyntax
  TransferSyntax::Rule_e getTSRule(void) const { return _tsRule; }

  void setTSRule(TransferSyntax::Rule_e use_rule) { _tsRule = use_rule; }

  // -- ------------------------------------------------- --
  // -- ASTypeDecoderAC abstract methods
  // -- ------------------------------------------------- --
  //Returns true if decoder able to decompose encoding of requested TransferSyntax
  virtual bool isTSsupported(TransferSyntax::Rule_e use_rule) const /*throw()*/ = 0;
  //Completely decodes type value from given encoding according to currently assigned TS rule
  virtual DECResult decode(const uint8_t * use_enc, TSLength max_len) /*throw(std::exception)*/ = 0;
};

template <class _TArg>
class ASTypeDecoderOf_T : public ASTypeDecoderAC {
public:
  ASTypeDecoderOf_T(TransferSyntax::Rule_e use_rule)
    : ASTypeDecoderAC(use_rule)
  { }
  virtual ~ASTypeDecoderOf_T()
  { }

  virtual void setValue(_TArg & use_value) = 0;
};

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_DEFS__ */

