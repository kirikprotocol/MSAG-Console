/* ************************************************************************* *
 * DER Encoder: 'TL' parts encoding methods.
 * ************************************************************************* */
#ifndef __ASN1_BER_TLV_ENCODER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_TLV_ENCODER

#include "eyeline/asn1/AbstractSyntax.hpp"
#include "eyeline/asn1/BER/rtutl/TypeTagging.hpp"
#include "eyeline/asn1/BER/rtutl/TLVProperties.hpp"
#include "eyeline/asn1/BER/rtutl/TSyntaxBER.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::TransferSyntax;
using eyeline::asn1::ASTypeEncoderAC;
using eyeline::asn1::ENCResult;

/* ************************************************************************* *
 * Abstract class that determines encoding properties of the ASN.1 type value 
 * and composes the V-part of BER/DER/CER encoding according to appropriate
 * clause from X.690.
 * ************************************************************************* */
class ValueEncoderIface {
protected:
  virtual ~ValueEncoderIface() {}

public:
  //Determines properties of addressed value encoding (LD form, constructedness)
  //according to requested encoding rule of BER family. Prepares data, necessary
  //for following encodeVAL() call.
  //NOTE.1: calculates length of value encoding if one of following conditions is
  //        fulfilled:    (LD form == ldDefinite) || ('calc_indef' == true)
  // 
  //NOTE.2: Throws in case of value that cann't be encoded.
  virtual void calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                            bool calc_indef = false) /*throw(std::exception)*/ = 0;

  //Encodes the addressed value according to previously calculated TLVProperties.
  //NOTE: Throws in case of value that cann't be encoded.
  virtual ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/ = 0;

  //Returns true if encoding of curr_rule is considered as valid encoding for tgt_rule
  virtual bool isPortable(TSGroupBER::Rule_e tgt_rule, TSGroupBER::Rule_e curr_rule) const /*throw()*/
  {
    return TSGroupBER::isPortable(tgt_rule, curr_rule);
  }
};


template <class _TArg>
class ValueEncoderOf_T : public ValueEncoderIface {
  // -- ------------------------------------------------- --
  // -- ValueEncoderIface interface methods 
  // -- ------------------------------------------------- --
  //virtual void calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
  //                          bool calc_indef = false) /*throw(std::exception)*/ = 0;
  //virtual ENCResult encodeVAL(uint8_t * use_enc,
  //                            TSLength max_len) const /*throw(std::exception)*/ = 0;

public:
  ValueEncoderOf_T()
  { }
  //
  virtual ~ValueEncoderOf_T()
  { }

  virtual void setValue(const _TArg & use_val) /*throw()*/ = 0;
};


/* ************************************************************************* *
 * Helper class that calculates and encodes the TLV encoding layout.
 * ************************************************************************* */
//'TL'-part composer
class TLComposer : public TLVStruct {
protected:
  uint8_t _octTag[MAX_IDENTIFIER_OCTS(ASTag::ValueType)]; //4 bytes as max for uint16_t
  uint8_t _octLOC[MAX_LDETERMINANT_OCTS(TSLength)];       //5 bytes as max for uint32_t

public:
  TLComposer() : TLVStruct()
  {
    _octTag[0] = _octLOC[0] = 0;
  }
  explicit TLComposer(const TLVProperty & use_prop)
    : TLVStruct(use_prop)
  {
    _octTag[0] = _octLOC[0] = 0;
  }
  ~TLComposer()
  { }

  //Calculates number of 'Tag', 'Length' and 'EOC' octets
  void calculate(const ASTag & use_tag);

  //Encodes 'begin-of-content' octets of TLV encoding
  ENCResult encodeBOC(uint8_t * use_enc, TSLength max_len) const;
  //Encodes 'end-of-content' octets of TLV encoding
  ENCResult encodeEOC(uint8_t * use_enc, TSLength max_len) const;
};

class TLVLayoutEncoder {
private:
  typedef eyeline::util::LWArray_T<TLComposer, uint8_t, _ASTaggingDFLT_SZ> TLComposersArray;

  //data for encoding optimization if layout calculation was done prior to encoding
  TLComposersArray  _tlws;
  TSLength          _szoBOC;  //overall length of 'begin-of-content' octets of TLV
                              //encoding, also serves as a 'calculation-performed' flag

//  TLVLayoutEncoder(const TLVLayoutEncoder&);
//  TLVLayoutEncoder& operator=(const TLVLayoutEncoder&);
protected:
  const ASTagging *   _effTags; //complete tagging of value type
  ValueEncoderIface * _valEnc;  //addressed value encoder, zero means layout isn't initialized
  TSGroupBER::Rule_e  _vRule;
  TLVProperty         _vProp;   //TLV-properties of addressed value encoding

  void resetLayout(void) { _tlws.clear(); _szoBOC = 0; }
  //Calculates TLV layout (tag & length octets + content octets + EOCs for each tag)
  const TLVStruct & calculateLayout(bool calc_indef = false);

  //Encodes 'TL'-part ('begin-of-content' octets)
  //NOTE: TLVLayout must be calculated.
  ENCResult encodeBOC(uint8_t * use_enc, TSLength max_len) const;
  //Encodes 'end-of-content' octets of encoding
  //NOTE: TLVLayout must be calculated.
  ENCResult encodeEOC(uint8_t * use_enc, TSLength max_len) const;

public:
  //Empty constructor: for later initialization
  explicit TLVLayoutEncoder(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : _szoBOC(0), _effTags(NULL), _valEnc(NULL), _vRule(use_rule)
  { }
  //'Generic type layout encoder' constructor.
  //NOTE: eff_tags must be a complete tagging of type!
  TLVLayoutEncoder(ValueEncoderIface & use_val_enc, const ASTagging * eff_tags,
                   TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : _szoBOC(0), _effTags(eff_tags), _valEnc(&use_val_enc), _vRule(use_rule)
  {
    if (_effTags)
      _tlws.enlarge(_effTags->size());
  }

  //
  bool isCalculated(void) const { return _szoBOC != 0; }
  //
  TSGroupBER::Rule_e  getVALRule(void) const { return _vRule; }
  //
  ValueEncoderIface * getVALEncoder(void) const { return _valEnc; }
  //
  const TLVProperty & getVALProperty(void) const { return _vProp; }

  //
  void setTagging(const ASTagging * use_tags)
  {
    if ((_effTags = use_tags) != 0)
      _tlws.enlarge(_effTags->size());
    resetLayout();
  }
  //
  void init(ValueEncoderIface & use_val_enc)
  {
    _valEnc = &use_val_enc; resetLayout();
  }
  //
  void init(ValueEncoderIface & use_val_enc, const ASTagging * use_tags)
  {
    _valEnc = &use_val_enc;
    setTagging(use_tags);
  }
  //
  void init(ValueEncoderIface & use_val_enc, const ASTagging * use_tags,
            TSGroupBER::Rule_e use_rule)
  {
    _valEnc = &use_val_enc;
    _vRule = use_rule;
    setTagging(use_tags);
  }

  void clear(void)
  {
    _valEnc = NULL; setTagging(NULL);
  }
  // -- ----------------------------------------------------- --
  // NOTE: all following methods may be called only after
  // ValueEncoderIface is set by constructor or init() call !!!
  // -- ----------------------------------------------------- --

  //Sets required kind of BER group encoding.
  //Returns: true if value encoding should be (re)calculated
  bool setVALRule(TSGroupBER::Rule_e use_rule) /*throw()*/
  {
    if (isCalculated() && !_valEnc->isPortable(use_rule, _vRule))
      resetLayout();
    _vRule = use_rule;
    return !isCalculated();
  }

  //Calculates TLV layout octets (length octets + EOC for each tag) basing
  //on encoding properties implied by addressed value.
  //Returns  TLVProperty for outermost tag.
  // 
  //NOTE: if 'calc_indef' is set, then full TLV encoding length is calculated
  //even if indefinite LD form is used. This may be required if this TLV is
  //enclosed by another that uses definite LD form.
  const TLVStruct & calculateTLV(bool calc_indef = false) /*throw(std::exception)*/;
  //Encodes by BER/DER/CER the previously calculated TLV layout (composes
  //complete TLV encoding).
  //NOTE: Throws in case the layout wasn't calculated.
  ENCResult encodeTLV(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/;
};

/* ************************************************************************* *
 * Basic abstract class that encodes by BER/DER/CER (composes TLV encoding)
 * the generic ASN.1 type.
 * ************************************************************************* */
class TypeEncoderAC : public ASTypeEncoderAC, public TypeTagging {
protected:
  TLVLayoutEncoder    _tlvEnc; //keeps reference to ValueEncoderIface

  //NOTE: refreshes tagging and initializes TLVLayoutEncoder
  const TLVStruct & prepareTLV(bool calc_indef) /*throw(std::exception)*/;

  TypeEncoderAC(const TypeEncoderAC & use_obj)
    : ASTypeEncoderAC(use_obj._tsRule), TypeTagging(use_obj)
    , _tlvEnc(TSGroupBER::getBERRule(use_obj._tsRule))
  {
    //NOTE: in case of CHOICE/Opentype the copying constructor of successsor
    //      MUST properly set options of TypeTagging
  }

public:
  //'Generic type encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  TypeEncoderAC(const ASTagging & eff_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : ASTypeEncoderAC(use_rule), TypeTagging(eff_tags)
    , _tlvEnc(TSGroupBER::getBERRule(use_rule))
  { }
  //'Untagged CHOICE/Opentype type encoder' constructor
  TypeEncoderAC(const TaggingOptions & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : ASTypeEncoderAC(use_rule), TypeTagging(&base_tags)
    , _tlvEnc(TSGroupBER::getBERRule(use_rule))
  { }
  //'Tagged Type encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  TypeEncoderAC(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                const ASTagging & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : ASTypeEncoderAC(use_rule), TypeTagging(use_tag, tag_env, base_tags)
    , _tlvEnc(TSGroupBER::getBERRule(use_rule))
  { }
  //'Tagged Type referencing untagged CHOICE/Opentype encoder' constructor
  TypeEncoderAC(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                const TaggingOptions & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : ASTypeEncoderAC(use_rule), TypeTagging(use_tag, tag_env, base_tags)
    , _tlvEnc(TSGroupBER::getBERRule(use_rule))
  { }
  //'Tagged Type referencing untagged CHOICE/Opentype encoder' constructor
  TypeEncoderAC(const ASTagging & use_tags, const TaggingOptions & base_tags,
               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : ASTypeEncoderAC(use_rule), TypeTagging(use_tags, base_tags)
    , _tlvEnc(TSGroupBER::getBERRule(use_rule))
  { }
  //
  virtual ~TypeEncoderAC()
  { }

  void init(ValueEncoderIface & use_venc) { _tlvEnc.init(use_venc); }

  ValueEncoderIface * getVALEncoder(void) const { return _tlvEnc.getVALEncoder(); }
  //
  TSGroupBER::Rule_e  getVALRule(void) const { return _tlvEnc.getVALRule(); }

  //Returns type tag that is used while sorting elements of SET,
  //depends on encoding rules being used.
  const ASTag * getOrdinalTag(void) const
  {
    return (getTSRule() == TransferSyntax::ruleCER) ? getCanonicalTag() : getTag();
  }

  //Compares two type encodings by their ordinal tags
  bool operator<(const TypeEncoderAC & cmp_tenc) const /*throw(std::exception)*/;

  // -- ------------------------------------------------- --
  // -- ASTypeEncoderAC virtual methods 
  // -- ------------------------------------------------- --
  //Sets required transferSyntax encoding rule
  //Returns: true if value encoding should be (re)calculated
  //Throws: in case of unsupported rule
  virtual bool setTSRule(TransferSyntax::Rule_e use_rule) /*throw(std::exception)*/
  {
    return ASTypeEncoderAC::setTSRule(use_rule) ? 
      _tlvEnc.setVALRule(TSGroupBER::getBERRule(use_rule)) : false;
  }

  // -- ------------------------------------------------- --
  // -- ASTypeEncoderAC abstract methods implementation
  // -- ------------------------------------------------- --
  virtual bool isTSsupported(TransferSyntax::Rule_e use_rule) const /*throw()*/
  { //TODO: CER isn't implemented yet
    //if (use_rule == TransferSyntax::ruleCER)
    //  return false; 
    return TSGroupBER::isBERSyntax(use_rule);
  }

  //Calculates length of resulted encoding without its composition.
  //NOTE: if calculation is impossible (f.ex. stream encoding) 
  //      ENCResult::encUnsupported is returned.
  virtual ENCResult calculate(void) /*throw(std::exception)*/;

  //Encodes by BER/DER/CER the TLV layout (composes complete TLV encoding).
  //NOTE: Throws in case of value that cann't be encoded.
  //NOTE: Calculates TLV layout if it wasn't calculated yet
  virtual ENCResult encode(uint8_t * use_enc, TSLength max_len) /*throw(std::exception)*/;
};


/* ************************************************************************* *
 * Abstract class that encodes by BER/DER/CER (composes TLV encoding) the
 * value of some ASN.1 type according to appropriate clause of X.690.
 * ************************************************************************* */
class TypeValueEncoderAC : public TypeEncoderAC, protected ValueEncoderIface {
private:
  using TypeEncoderAC::init;

protected:
  // -- -------------------------------------- --
  // -- ValueEncoderIface interface methods
  // -- -------------------------------------- --
  //virtual void calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
  //                          bool calc_indef = false) /*throw(std::exception)*/ = 0;
  //virtual ENCResult encodeVAL(uint8_t * use_enc,
  //                            TSLength max_len) const /*throw(std::exception)*/ = 0;
  //virtual bool isPortable(TSGroupBER::Rule_e tgt_rule, TSGroupBER::Rule_e curr_rule) const /*throw()*/; 

  TypeValueEncoderAC(const TypeValueEncoderAC & use_obj)
    : TypeEncoderAC(use_obj)
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
    //NOTE: in case of CHOICE/Opentype the copying constructor of successsor
    //      MUST properly set options of TypeTagging
  }
public:
  //'Generic type value encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  TypeValueEncoderAC(const ASTagging & eff_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(eff_tags, use_rule)
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //'Generic tagged type value encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  TypeValueEncoderAC(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                     const ASTagging & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(use_tag, tag_env, base_tags, use_rule)
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //'Untagged CHOICE/Opentype type value encoder' constructor
  TypeValueEncoderAC(const TaggingOptions & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(base_tags, use_rule)
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //'Tagged Type referencing untagged CHOICE/Opentype value encoder' constructor
  TypeValueEncoderAC(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                     const TaggingOptions & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(use_tag, tag_env, base_tags, use_rule)
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //'Tagged Type referencing untagged CHOICE/Opentype value encoder' constructor
  TypeValueEncoderAC(const ASTagging & use_tags, const TaggingOptions & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(use_tags, base_tags, use_rule)
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //
  virtual ~TypeValueEncoderAC()
  { }
};


template <
    class _TArg
  , class _ValueEncoderTArg /* : public ValueEncoderOf_T<_TArg>*/
>
class TypeValueEncoderOf_T : public TypeEncoderAC, public _ValueEncoderTArg {
private:
  using TypeEncoderAC::init;

public:
  //'Generic type value encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  TypeValueEncoderOf_T(const ASTagging & eff_tags,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(eff_tags, use_rule)
    , _ValueEncoderTArg()
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //'Generic tagged type value encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  TypeValueEncoderOf_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                     const ASTagging & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(use_tag, tag_env, base_tags, use_rule)
    , _ValueEncoderTArg()
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //'Untagged CHOICE/Opentype type value encoder' constructor
  TypeValueEncoderOf_T(const TaggingOptions & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(base_tags, use_rule)
    , _ValueEncoderTArg()
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //'Tagged Type referencing untagged CHOICE/Opentype value encoder' constructor
  TypeValueEncoderOf_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                     const TaggingOptions & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(use_tag, tag_env, base_tags, use_rule)
    , _ValueEncoderTArg()
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //'Tagged Type referencing untagged CHOICE/Opentype value encoder' constructor
  TypeValueEncoderOf_T(const ASTagging & use_tags, const TaggingOptions & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(use_tags, base_tags, use_rule)
    , _ValueEncoderTArg()
  {
    TypeEncoderAC::init(*(ValueEncoderIface*)this);
  }
  //
  virtual ~TypeValueEncoderOf_T()
  { }
};


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_ENCODER */

