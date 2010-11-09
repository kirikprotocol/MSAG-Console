/* ************************************************************************* *
 * BER Encoder: SEQUENCE OF type encoder (linked represenattion).
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_SEQUENCE_OF_LINKED
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_SEQUENCE_OF_LINKED

#include <list>
#include "eyeline/asn1/BER/rtenc/EncodeSeqOf.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************* *
 * Template class of encoder of SEQUENCE OF with 'Linked'
 * data structure representation.
 * ************************************************************* */
template <
    //Must have default and copying constructors
    class _TArg
    //Must have tagged/untagged and copying constructors
  , class _EncoderOfTArg /* : public TypeValueEncoder_T<_TArg>*/
  , uint16_t _NumElemsTArg = 2 //estimated number of element values
>
class EncoderOfSeqOfLinked_T : public EncoderOfSeqOfAC_T<_TArg, _EncoderOfTArg, _NumElemsTArg> {
protected:
  //constructor for types defined as SEQUENCE OF with own tagging
  EncoderOfSeqOfLinked_T(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : EncoderOfSeqOfAC_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(eff_tags, use_rule)
  { }

public:
  typedef std::list<_TArg> ElementsList;
  typedef std::list<_TArg *> ElementsPtrList;

  // constructor for untagged SEQUENCE OF
  explicit EncoderOfSeqOfLinked_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : EncoderOfSeqOfAC_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(use_rule)
  { }
  // constructor for tagged SEQUENCE OF
  EncoderOfSeqOfLinked_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : EncoderOfSeqOfAC_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(use_tag, tag_env, use_rule)
  { }
  EncoderOfSeqOfLinked_T(const EncoderOfSeqOfLinked_T & use_obj)
    : EncoderOfSeqOfAC_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(use_obj)
  { }
  //
  virtual ~EncoderOfSeqOfLinked_T()
  { }

  // ----------------------------------------------------------
  // Initialization methods
  // ----------------------------------------------------------
  //void setElementTag(const ASTag & elm_tag, ASTagging::Environment_e tag_env) /*throw()*/;
  //void setMaxElements(uint16_t max_elems) /*throw()*/;

  void setValue(const ElementsList & use_list) /*throw(std::exception)*/
  {
    uint16_t last_idx = 0;
    if (!use_list.empty()) {
      reserveElementEncoders(use_list.size()); //throws
      for(typename ElementsList::const_iterator it = use_list.begin(); it != use_list.end(); ++it, ++last_idx)
        addElementValue(*it); //throws
    }
    this->clearElements(last_idx);
  }
  //
  void setValue(const ElementsPtrList & use_list) /*throw(std::exception)*/
  {
    uint16_t last_idx = 0;
    if (!use_list.empty()) {
      reserveElementEncoders(use_list.size()); //throws
      for(typename ElementsPtrList::const_iterator it = use_list.begin(); it != use_list.end(); ++it)
        if (*it) {
          addElementValue(**it); //throws
          ++last_idx;
        }
    }
    this->clearElements(last_idx);
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_SEQUENCE_OF_LINKED */

