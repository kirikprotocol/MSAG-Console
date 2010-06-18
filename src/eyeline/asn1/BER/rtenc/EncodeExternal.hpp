/* ************************************************************************* *
 * BER Encoder: EXTERNAL encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_EXTERNAL
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_EXTERNAL

#include "eyeline/asn1/ASExternal.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeEOID.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeObjDescr.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeStruct.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeEmbdEnc.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

//According to X.690 cl.8.18.1,
//EXTERNAL type is represented by SEQUENCE with following definition
//
//[UNIVERSAL 8] IMPLICIT SEQUENCE {
//           direct-reference       OBJECT IDENTIFIER OPTIONAL,
//           indirect-reference     INTEGER OPTIONAL,
//           data-value-descriptor  ObjectDescriptor OPTIONAL,
//           encoding               EmbeddedEncoding
//          }

class EncoderOfExternal : public EncoderOfStructure_T<4> {
private:
  //EXTERNAL has predefined structure, so hide this methods
  using EncoderOfStructure_T<4>::addField;
  using EncoderOfStructure_T<4>::setField;

  union {
    void *    aligner;
    uint8_t   buf[sizeof(EncoderOfEOID)];
  }   _memDRef;

  union {
    void *    aligner;
    uint8_t   buf[sizeof(EncoderOfINTEGER)];
  }   _memIRef;

  union {
    void *    aligner;
    uint8_t   buf[sizeof(EncoderOfObjDescriptor)];
  }   _memDescr;

protected:
  EncoderOfEmbdEncoding     _encEnc;
  //optional fields
  EncoderOfEOID *           _encDRef;
  EncoderOfINTEGER *        _encIRef;
  EncoderOfObjDescriptor *  _encDescr;

  // constructor for encoder of tagged type referencing EXTERNAL
  EncoderOfExternal(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfStructure_T<4>(eff_tags, use_rule)
    , _encDRef(0), _encIRef(0), _encDescr(0)
  {
    _memDescr.aligner = _memDRef.aligner = _memIRef.aligner =  0;
  }

public:
  // constructor for encoder of EXTERNAL
  EncoderOfExternal(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfStructure_T<4>(asn1::_tagsEXTERNAL, use_rule)
    , _encDRef(0), _encIRef(0), _encDescr(0)
  {
    _memDescr.aligner = _memDRef.aligner = _memIRef.aligner =  0;
  }
  // constructor for encoder of tagged EXTERNAL
  EncoderOfExternal(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfStructure_T<4>(use_tag, tag_env, asn1::_tagsEXTERNAL, use_rule)
    , _encDRef(0), _encIRef(0), _encDescr(0)
  {
    _memDescr.aligner = _memDRef.aligner = _memIRef.aligner =  0;
  }

  ~EncoderOfExternal();

  void setValue(const asn1::ASExternal & val_ext) /*throw(std::exception)*/;
  //
  void setValue(const EncodedOID & as_oid, TypeEncoderAC & type_enc, const char * descr = NULL)
    /*throw(std::exception)*/;
  //
  void setValue(int32_t prs_ctx, TypeEncoderAC & type_enc, const char * descr = NULL)
    /*throw(std::exception)*/;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_EXTERNAL */

