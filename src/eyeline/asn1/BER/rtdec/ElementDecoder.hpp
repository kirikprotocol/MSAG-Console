/* ************************************************************************* *
 * BER Decoder: base class of all element decoders of structured types
 * (SEQUENCE, SET, CHOICE, SEQUENCE OF, SET OF).
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_ELEMENTS_DECODER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_ELEMENTS_DECODER

#include "util/Exception.hpp"
#include "eyeline/asn1/BER/rtdec/EDAlternative.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

typedef eyeline::util::LWArrayExtension_T<EDAlternative, uint16_t> EDAlternativesArray;

class ElementDecoderAC {
public:
  enum TDState_e { //TagDecoder state
      tdEmpty = 0   //
    , tdInit        //TagDecoder is being initialized
    , tdBuilt       //TagDecoder is built and ready for use
    , tdProcess     //TagDecoder is in process of decoding
  };

  enum EDAStatus_e { //Element Alternative determination status
      edaOk = 0
    , edaUnknown          //unknown alternative found or TagDecoder isn't built yet
    , edaMisplaced        //misplaced alternative
    , edaMultiple         //multiply occured alternative
    , edaMisplacedExt     //misplaced unknown extension
    , edaMisssed          //at least one mandatory element is missed
  };

  struct EDAResult { //Element Alternative determination result
    EDAStatus_e           _status;
    const EDAlternative * _alt;

    explicit EDAResult(EDAStatus_e use_status = ElementDecoderAC::edaOk,
                       const EDAlternative * use_alt = 0)
      : _status(use_status), _alt(use_alt)
    { }
    ~EDAResult()
    { }

    bool isOk(void) const { return (_status == ElementDecoderAC::edaOk); }
  };

  //
  class DuplicateUIdException : public smsc::util::Exception {
  public:
    DuplicateUIdException(uint16_t unique_id)
      : smsc::util::Exception("ber::ElementDecoderAC::setAlternative(): "
                              "alternative[%u] is already defined", unique_id)
    { }
  };
  //
  class UndefinedUIdException : public smsc::util::Exception {
  public:
    UndefinedUIdException(uint16_t unique_id)
      : smsc::util::Exception("ber::ElementDecoderAC::buildTagDecoder(): "
                              "alternative[%u] is not defined", unique_id)
    { }
  };
  //
  class DuplicateTagException : public smsc::util::Exception {
  public:
    DuplicateTagException(const ASTag & use_tag, uint16_t alt_uid1, uint16_t alt_uid2)
      : smsc::util::Exception("ber::ElementDecoderAC::buildTagDecoder(): "
                              "duplicate tag '%s' of alternatives %u, %u",
                              use_tag.toString().c_str(), alt_uid1, alt_uid2)
    { }
  };
  //
  class AmbiguousCtxException : public smsc::util::Exception {
  public:
    AmbiguousCtxException(uint16_t alt_uid)
      : smsc::util::Exception("ber::ElementDecoderAC::buildTagDecoder(): "
                              "OpenType alternative %u is used in ambiguous context",
                              alt_uid)
    { }
  };

protected:
  TDState_e   _tdState;
  //ElementDecoderAC initialization data
  bool        _extensible;  //elements order is extensible - unknown elements
                            //may be present as special pseudo-alternative

  EDAlternativesArray *   _altsArr; //ElementAlternatives storage, actually it's
                                    //just a reference to a successor member, so
                                    //its copying constructor MUST properly set
                                    //that pointer.
  // ----------------------------------------------------------
  // Initialization methods for successors
  // ----------------------------------------------------------
  void insertAlternative(const EDAlternative & use_alt) /* throw(std::exception)*/;

  void setAltStorage(EDAlternativesArray & alt_store) { _altsArr = &alt_store; }

  //NOTE: copying constructor of successsor MUST properly set _alrArr
  ElementDecoderAC(const ElementDecoderAC & use_obj)
    : _tdState(use_obj._tdState), _extensible(use_obj._extensible), _altsArr(0)
  { }

  explicit ElementDecoderAC(EDAlternativesArray & alt_store)
    : _tdState(tdEmpty), _extensible(false), _altsArr(&alt_store)
  { }

  // ----------------------------------------------------------
  // ElementDecoderAC Interface methods (protected)
  // ----------------------------------------------------------
  //Returns blocking mode for alternative with given UId
  virtual uint8_t getBlocking(uint16_t alt_uid) const /*throw(std::exception)*/ = 0;
  //Generates TagDecoder initialization data
  virtual void buildTagDecoder(void) /* throw(std::exception)*/ = 0;
  //Reverts TagDecoder runtime data to its 'just-built' state
  virtual void resetTagDecoder(void) /*throw()*/ = 0;
  //Destroys all TagDecoder data
  virtual void eraseTagDecoder(void) /*throw()*/ = 0;
  //Checks if occurence of alternative with given tag is legal at current
  //ElementDecoderAC state updating it if necessary.
  virtual EDAResult processElementTag(const ASTag & use_tag) /*throw(std::exception)*/ = 0;
  //Verifies that ElementDecoderAC is in complete state - all mandatory
  //alternatives are processed according to its decoding order.
  virtual EDAResult verifyTDCompletion(void) const /*throw()*/ = 0;

public:
  virtual ~ElementDecoderAC()
  { }

  // ----------------------------------------------------------
  // Initialization methods
  // ----------------------------------------------------------

  //Sets tagged alternative of some type
  void setAlternative(uint16_t unique_idx, const ASTag & use_tag,
                      ASTagging::Environment_e tag_env,
                      EDAlternative::Presence_e use_presence)
    /* throw(std::exception)*/
  {
    insertAlternative(EDAlternative(unique_idx, use_tag, tag_env, use_presence));
  }
  //Sets untagged alternative of ordinary type
  void setAlternative(uint16_t unique_idx, const ASTag & use_tag,
                      EDAlternative::Presence_e use_presence)
    /* throw(std::exception)*/
  {
    insertAlternative(EDAlternative(unique_idx, use_tag, use_presence));
  }
  //Sets untagged alternative of ANY/Opentype
  void setAlternative(uint16_t unique_idx, EDAlternative::Presence_e use_presence)
    /* throw(std::exception)*/
  {
     insertAlternative(EDAlternative(unique_idx, use_presence));
  }

  //Sets untagged alternative of untagged CHOICE type
  void setAlternative(uint16_t unique_idx, const TaggingOptions & use_tag_opts,
                      EDAlternative::Presence_e use_presence)
    /* throw(std::exception)*/
  {
     insertAlternative(EDAlternative(unique_idx, use_tag_opts.getMap(),use_presence));
  }
  //Sets alternative for unknown extension additions entry
  void setUnkExtension(uint16_t unique_idx) /* throw(std::exception)*/
  {
    insertAlternative(EDAlternative(unique_idx));
  }

  // ----------------------------------------------------------
  // Runtime processing methods
  // ----------------------------------------------------------

  //Checks if occurence of alternative with given tag is legal at current
  //ElementDecoderAC state updating it if necessary.
  EDAResult processElement(const ASTag & use_tag) /*throw(std::exception)*/;

  //Verifies that ElementDecoderAC is in complete state - all mandatory
  //alternatives are processed according to its decoding order.
  EDAResult verifyCompletion(void) const /*throw()*/
  {
    return (_tdState <= tdBuilt) ? EDAResult(edaUnknown) : verifyTDCompletion();
  }

  //Reverts ElementDecoderAC to its initialized state, making it ready for next decoding.
  void reset(void) /*throw()*/;

  //ElementDecoderAC to its 'just-allocated' state, erases all runtime and
  //initialization data.
  void erase(void) /*throw()*/;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_ELEMENTS_DECODER */

