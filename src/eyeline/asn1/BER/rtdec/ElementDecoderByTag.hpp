/* ************************************************************************* *
 * BER Decoder: SET/CHOICE types type element decoder, that uses element's
 *              tags as ordering factor.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_ELEMENTS_DECODER_BY_TAG
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_ELEMENTS_DECODER_BY_TAG

#include "eyeline/asn1/BER/rtdec/ElementDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

//ElementDecoder of SET or CHOICE types
class ElementDecoderByTagAC : public ElementDecoderAC {
public:
  enum Order_e {
      orderNone = 0       //alternatives may occur in arbitrary order
    , orderDistingueshed  //alternatives are ordered according to canonical
                          //order of their distingueshed tags
    , orderCanonical      //alternatives are ordered according to canonical
                          //order of their canonical tags
  };

protected:
  class TagOccurence {
  protected:
    const EDAlternative * _alt;
    TagOccurence *        _occCan;      //occurence of canonical alternative of CHOICE,
                                        //that keeps counter of all CHOICE options
  public:
    bool                  _blocked;
    bool                  _optional;
    uint16_t              _count;
    uint16_t              _optCount;    //counter of other CHOICE alternatives,
                                        //meaningfull only if this occurence is
                                        //associated with canonical alternative
    TagOccurence()
      : _alt(0), _occCan(0), _blocked(false), _optional(true)
      , _count(0), _optCount(0)
    { }
    TagOccurence(const EDAlternative & use_alt, TagOccurence * occ_can = 0)
      : _alt(&use_alt), _occCan(occ_can), _blocked(false)
      , _optional(!occ_can ? use_alt.isOptional() : true)
      , _count(0), _optCount(0)
    { }
    ~TagOccurence()
    { }

    void init(const EDAlternative & use_alt, TagOccurence * occ_can = 0)
    {
      _alt = &use_alt; _occCan = occ_can; reset();
    }
    void setCanonical(TagOccurence * occ_can) { _occCan = occ_can; }

    //NOTE: may be called only for initialzed obj
    void reset(void)
    {
      _blocked = false; _count = _optCount = 0;
      _optional = (_alt && isCanonical()) ? _alt->isOptional() : true;
    }
    //Returns true if occurence is associated with canonical tag of element
    bool isCanonical(void) const { return (_occCan == 0) ; }
    //
    TagOccurence * getCanonical(void) const { return _occCan; }
    //
    bool isAvailable(bool only_canonical = false) const
    {
      return _blocked ? false : (only_canonical ? isCanonical() : true);
    }
    //
    uint16_t numOccured(bool all_options = false) const
    {
      return !all_options ? _count : 
                            (_occCan ? _occCan->numOccured(true) : _count + _optCount);
    }
    //
    const EDAlternative * getAlt(void) const { return _alt; }
    //Returns unique_id of associated alternative
    uint8_t getAltUId(void) const { return _alt->getUId(); }
  };

  typedef std::map<ASTag, TagOccurence> EDATagsOrder;

  //ElementDecoderByTagAC initialization data
  Order_e                 _order;
  //runtime data
  EDATagsOrder            _orderTags;   //registry of all alternatives tags (distingueshed order)

  TagOccurence * insertTag(const ASTag & use_tag, const TagOccurence & alt_occ)
    /* throw(std::exception)*/;

  //Updates registry of alternative's tags
  void updateTagsRegistry(const EDAlternative & use_alt) /* throw(std::exception)*/;

  //
  EDAStatus_e blockAlternativeTags(EDATagsOrder::iterator & it_tag, bool only_canonical)
    /*throw()*/;
  //
  EDAStatus_e blockPreceedingByTag(EDATagsOrder::iterator it_tag);
  //
  EDAStatus_e blockFollowingByTag(EDATagsOrder::iterator it_tag);
  //
  EDAStatus_e processAlternative(EDATagsOrder::iterator & it_tag)
    /*throw(std::exception)*/;

  // ----------------------------------------------------------
  // ElementDecoderAC Interface methods (protected)
  // ----------------------------------------------------------
  //Returns blocking mode for alternative with given UId
  virtual uint8_t getBlocking(uint16_t alt_uid) const /*throw(std::exception)*/ = 0;
  //Generates TagDecoder initialization data
  virtual void buildTagDecoder(void) /* throw(std::exception)*/;
  //Reverts TagDecoder runtime data to its 'just-built' state
  virtual void resetTagDecoder(void) /*throw()*/;
  //Destroys all TagDecoder data
  virtual void eraseTagDecoder(void) /*throw()*/;
  //Checks if occurence of alternative with given tag is legal at current
  //ElementDecoderAC state updating it if necessary.
  virtual EDAResult processElementTag(const ASTag & use_tag) /*throw(std::exception)*/;
  //Verifies that ElementDecoderAC is in complete state - all mandatory
  //alternatives are processed according to its decoding order.
  virtual EDAResult verifyTDCompletion(void) const /*throw()*/;

  //NOTE: copying constructor of successsor MUST properly set _alrArr
  ElementDecoderByTagAC(const ElementDecoderByTagAC & use_obj)
    : ElementDecoderAC(use_obj), _order(use_obj._order)
    , _orderTags(use_obj._orderTags)
  { }

public:
  ElementDecoderByTagAC(EDAlternativesArray & alt_store, Order_e use_order = orderNone)
    : ElementDecoderAC(alt_store), _order(use_order)
  { }
  ~ElementDecoderByTagAC()
  { }

  Order_e getOrder(void) const { return _order; }

  //NOTE: changing Order destroys all ElemnentDecoder processing data!
  void setOrder(Order_e use_order)
  {
    _order = use_order;
    reset();
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_ELEMENTS_DECODER_BY_TAG */

