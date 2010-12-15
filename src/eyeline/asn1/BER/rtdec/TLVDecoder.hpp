/* ************************************************************************* *
 * BER Decoder: 'TLV' encoding decoder definitions.
 * ************************************************************************* */
#ifndef __ASN1_BER_TLV_DECODER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_TLV_DECODER

#include "eyeline/asn1/AbstractSyntax.hpp"
#include "eyeline/asn1/BER/rtutl/TypeTagging.hpp"
#include "eyeline/asn1/BER/rtutl/TLVUtils.hpp"
#include "eyeline/asn1/BER/rtutl/TSyntaxBER.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::TSLength;
using eyeline::asn1::DECResult;
using eyeline::asn1::TransferSyntax;
using eyeline::asn1::ASTypeDecoderAC;

//Searches 'content octets' of primitive encoding for EOC octets (two zeroes)
//Returns number of bytes preceeding EOC if it's found
//
// T,L, COC, EOC
//    ^ --> ^
extern DECResult searchEOC(const uint8_t * use_enc, TSLength max_len);
//Searches 'content octets' of constructed encoding for outermost EOC octets (two zeroes)
//Returns number of bytes preceeding outermost EOC if it's found
//
// T,L
// --> T,L, COC, EOC
//  |  T,L, COC, EOC
// --> T,L, COC, EOC
// EOC 
extern DECResult searchEOCconstructed(const uint8_t * use_enc, TSLength max_len, bool relaxed_rule = true);

//Returns number of bytes rest of TLV occupies, basing on previously
//decoded outer TL-pair.
//
// T,L, COC, EOC
//     ^ -->   ^
extern DECResult skipTLV(const TLParser & outer_tl, const uint8_t * use_enc,
                         TSLength max_len, bool relaxed_rule = true);

//Returns number of bytes whole TLV occupies.
//
// T,L, COC, EOC
// ^    -->    ^
extern DECResult skipTLV(const uint8_t * use_enc, TSLength max_len, bool relaxed_rule = true);
//Returns true if EOC present at given encoding position
extern bool checkEOC(const uint8_t * use_enc, TSLength max_len) /*throw()*/;

/* ************************************************************************* *
 * Abstract class that decodes BER/DER/CER encoding of the generic ASN.1 type
 * value according to appropriate clause from X.690.
 * ************************************************************************* */
class ValueDecoderIface {
protected:
  virtual ~ValueDecoderIface() { }

public:
  // -----------------------------------------------------------
  // -- ValueDecoderIface interface methods
  // -----------------------------------------------------------
  virtual DECResult decodeVAL(const TLVProperty * val_prop,
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false)
    /*throw(std::exception)*/ = 0;
};

template <class _TArg>
class ValueDecoderOf_T : protected ValueDecoderIface {
protected:
  _TArg * _dVal;

  // -- -------------------------------------------------- --
  // -- ValueDecoderIface abstract methods are to implement
  // -- -------------------------------------------------- --
  virtual DECResult decodeVAL(const TLVProperty * val_prop,
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false)
    /*throw(std::exception)*/;

public:
  explicit ValueDecoderOf_T(_TArg * use_val = 0)
    : _dVal(use_val)
  { }
  ~ValueDecoderOf_T()
  { }

  void setValue(_TArg & use_val) { _dVal = &use_val; }
};

/* ************************************************************************* *
 * Basic abstract class that decodes by BER/DER/CER (decomposes TLV encoding)
 * the generic ASN.1 type.
 * ************************************************************************* */
class TypeDecoderAC : public ASTypeDecoderAC, public TypeTagging {
protected:
  bool                _relaxedRule;
  ValueDecoderIface * _valDec;  //NOTE: it may be just a reference to a successor
                                //memer, so its copying constructor should care
                                //about proper _valDec setting.
  const TLParser *    _outerTL;

  //NOTE.1: in case of CHOICE/Opentype the copying constructor of successsor
  //        MUST properly set _optTags  by TypeTagging::setOptions().
  //NOTE.2: the copying constructor of successsor MUST properly set _valDec
  //        by calling init()
  TypeDecoderAC(const TypeDecoderAC & use_obj)
    : ASTypeDecoderAC(use_obj), TypeTagging(use_obj)
    , _relaxedRule(use_obj._relaxedRule), _valDec(0)
    , _outerTL(use_obj._outerTL)
  { }

  //'Generic type decoder' constructor
  // NOTE: eff_tags is a complete effective tagging of type!
  explicit TypeDecoderAC(const ASTagging & eff_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : ASTypeDecoderAC(use_rule), TypeTagging(eff_tags)
    , _relaxedRule(false), _valDec(0), _outerTL(0)
  { }
  //'Untagged CHOICE/Opentype type decoder' constructor
  explicit TypeDecoderAC(const TaggingOptions * base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : ASTypeDecoderAC(use_rule), TypeTagging(base_tags)
    , _relaxedRule(false), _valDec(0), _outerTL(0)
  { }
  //'Tagged Type decoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  TypeDecoderAC(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                const ASTagging & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : ASTypeDecoderAC(use_rule), TypeTagging(use_tag, tag_env, base_tags)
    , _relaxedRule(false), _valDec(0), _outerTL(0)
  { }
  //'Tagged Type referencing untagged CHOICE/Opentype decoder' constructor
  TypeDecoderAC(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                const TaggingOptions & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : ASTypeDecoderAC(use_rule), TypeTagging(use_tag, tag_env, base_tags)
    , _relaxedRule(false), _valDec(0), _outerTL(0)
  { }
  //'Tagged Type referencing untagged CHOICE/Opentype decoder' constructor
  TypeDecoderAC(const ASTagging & use_tags, const TaggingOptions & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : ASTypeDecoderAC(use_rule), TypeTagging(use_tags, base_tags)
    , _relaxedRule(false), _valDec(0), _outerTL(0)
  { }

  void init(ValueDecoderIface & use_vdec) { _valDec = &use_vdec; }

public:
  //
  virtual ~TypeDecoderAC()
  { }
  //
  void setRelaxedRule(bool relaxed_rule) { _relaxedRule = relaxed_rule; }
  //
  void setOutermostTL(const TLParser * outer_tl) { _outerTL  = outer_tl; }
  //
  TSGroupBER::Rule_e getVALRule(void) const /*throw(std::exception)*/
  {
    return TSGroupBER::getBERRule(getTSRule());
  }

  // -- ------------------------------------------------- --
  // -- ASTypeDecoderAC abstract methods implementation
  // -- ------------------------------------------------- --
  virtual bool isTSsupported(TransferSyntax::Rule_e use_rule) const /*throw()*/
  {
    return TSGroupBER::isBERSyntax(use_rule);
  }
  //
  virtual DECResult decode(const uint8_t * use_enc, TSLength max_len) /*throw(std::exception)*/;
};


/* ************************************************************************* *
 * Abstract class that decodes by BER/DER/CER (decomposes TLV encoding) the
 * value of some ASN.1 type according to appropriate clause of X.690.
 * ************************************************************************* */
class TypeValueDecoderAC : public TypeDecoderAC, protected ValueDecoderIface {
protected:
  // -- ************************************************* --
  // -- ValueDecoderIface abstract methods are to implement
  // -- ************************************************* --
  //virtual DECResult decodeVAL(const TLVProperty * val_prop,
  //                            const uint8_t * use_enc, TSLength max_len,
  //                            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
  //                            bool relaxed_rule = false)
  //  /*throw(std::exception)*/ = 0;


  //NOTE: in case of CHOICE/Opentype the copying constructor of successsor
  //      MUST properly set _optTags  by setOptions().
  TypeValueDecoderAC(const TypeValueDecoderAC & use_obj)
    : TypeDecoderAC(use_obj)
  {
    TypeDecoderAC::init(*(ValueDecoderIface*)this);
  }

public:
  //'Generic type decoder' constructor
  // NOTE: eff_tags is a complete effective tagging of type!
  explicit TypeValueDecoderAC(const ASTagging & eff_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeDecoderAC(eff_tags, use_rule)
  {
    TypeDecoderAC::init(*(ValueDecoderIface*)this);
  }
  //'Untagged CHOICE/Opentype type decoder' constructor
  explicit TypeValueDecoderAC(const TaggingOptions * base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeDecoderAC(base_tags, use_rule)
  {
    TypeDecoderAC::init(*(ValueDecoderIface*)this);
  }
  //'Tagged Type decoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  TypeValueDecoderAC(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                const ASTagging & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeDecoderAC(use_tag, tag_env, base_tags, use_rule)
  {
    TypeDecoderAC::init(*(ValueDecoderIface*)this);
  }
  //'Tagged Type referencing untagged CHOICE/Opentype decoder' constructor
  TypeValueDecoderAC(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                const TaggingOptions & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeDecoderAC(use_tag, tag_env, base_tags, use_rule)
  {
    TypeDecoderAC::init(*(ValueDecoderIface*)this);
  }
  //'Tagged Type referencing untagged CHOICE/Opentype decoder' constructor
  TypeValueDecoderAC(const ASTagging & use_tags, const TaggingOptions & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeDecoderAC(use_tags, base_tags, use_rule)
  {
    TypeDecoderAC::init(*(ValueDecoderIface*)this);
  }
  //
  virtual ~TypeValueDecoderAC()
  { }
};

/* -------------------------------------------------- *
 * UNIVERSAL Type value decoder template
 * -------------------------------------------------- */
template <class _TArg, const ASTagging & _uniTags_TArg>
class UNITypeValueDecoder_T : public TypeDecoderAC, public ValueDecoderOf_T<_TArg> {
private:
  using TypeDecoderAC::init;

protected:
  //using ValueDecoderOf_T<_TArg>::decodeVAL;

public:
  //Constructor for UNIVERSAL type value decoder
  explicit UNITypeValueDecoder_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeDecoderAC(_uniTags_TArg, use_rule)
  {
    TypeDecoderAC::init(*(ValueDecoderIface*)this);
  }
  //Constructor for tagged UNIVERSAL type value decoder
  UNITypeValueDecoder_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                   TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeDecoderAC(use_tag, tag_env, _uniTags_TArg, use_rule)
  {
    TypeDecoderAC::init(*(ValueDecoderIface*)this);
  }
  virtual ~UNITypeValueDecoder_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_DECODER */

