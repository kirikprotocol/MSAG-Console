/* ************************************************************************* *
 * BER Decoder: Structured type element decoder (uses ASN.1 textual
 *              definition as ordering factor).
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_ELEMENTS_DECODER_BY_TYPEDEF
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_ELEMENTS_DECODER_BY_TYPEDEF

#include "eyeline/util/BITArray.hpp"
#include "eyeline/asn1/BER/rtdec/ElementDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class ElementDecoderByTDef : public ElementDecoderAC {
protected:
  struct EDAOccurence { //ElementDecoder alternative occurence data
    bool            _isSingle;  //only single occurence of element is allowed
    uint16_t        _tdRowIdx;  //index of associated TagDecoder matrix row

    EDAOccurence() : _isSingle(true), _tdRowIdx(0)
    { }
    ~EDAOccurence()
    { }

    bool  isBlocked(bool is_occured) const
    {
      return _isSingle ? is_occured : false;
    }
  };

  typedef smsc::core::buffers::LWArrayExtension_T<EDAOccurence, uint16_t> EDAOccurenceArray;
  typedef eyeline::util::BITArrayExtension_T<uint16_t> EDAOccurencesMask;

  //TagDecoder entry
  struct TDEntry {
    ASTag     _tag;
    uint16_t  _altIdx;  //index of associated alternative datat structures in storages

    TDEntry() : _altIdx(0)
    { }
    TDEntry(const ASTag & use_tag, uint16_t use_idx)
      : _tag(use_tag), _altIdx(use_idx)
    { }

    bool operator< (const TDEntry & cmp_obj) const
    {
      return _tag < cmp_obj._tag;
    }
  };
  //
  typedef smsc::core::buffers::LWArray_T<TDEntry, uint16_t, 1> TDMatrixRow;
  typedef smsc::core::buffers::LWArrayExtension_T<TDMatrixRow, uint16_t> TDMatrix;

  //Initialization data

  //NOTE: actually all arrays are just a references to a members of successor,
  //      so the copying constructor of successsor MUST properly set them.
  EDAOccurenceArray   * _occArr;
  TDMatrix            * _tdMatrix;
  //Runtime data
  EDAOccurencesMask   * _occMask;
  uint16_t              _fieldIdx;  //index of next field is to decode

  //NOTE: throws in case of multiple tag occurences detected
  void addTag2Row(TDMatrixRow & td_row, const ASTag & use_tag, uint16_t alt_idx)
      /*throw(std::exception)*/;
  //NOTE: throws in case of multiple tag occurences detected
  void addTags2Row(TDMatrixRow & td_row, const EDAlternative & use_alt)
      /*throw(std::exception)*/;
  //Returns index of next alternative is to process
  //NOTE: throws in case of multiple tag occurences detected
  uint16_t buildTDMatrixRow(uint16_t row_idx, uint16_t start_idx)
      /*throw(std::exception)*/;

  void setStorages(EDAlternativesArray & use_alt_arr, EDAOccurenceArray & use_occ_arr,
                   TDMatrix & use_matrix, EDAOccurencesMask & use_occ_mask)
  {
    _altsArr = &use_alt_arr;
    _occArr = &use_occ_arr;
    _tdMatrix = &use_matrix;
    _occMask = &use_occ_mask;
  }

  //NOTE: copying constructor of successsor MUST call setStorages()
  ElementDecoderByTDef(const ElementDecoderByTDef & use_obj)
    : ElementDecoderAC(use_obj)
    , _occArr(0), _tdMatrix(0), _occMask(0)
    , _fieldIdx(use_obj._fieldIdx)
  { }

  // ----------------------------------------------------------
  // ElementDecoderAC interface methods
  // ----------------------------------------------------------
  //Returns blocking mode for alternative with given UId
  virtual uint8_t getBlocking(uint16_t alt_uid) const /*throw(std::exception)*/;
  //Generates TagDecoder initialization data
  virtual void buildTagDecoder(void)  /* throw(std::exception)*/;
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

public:
  ElementDecoderByTDef(EDAlternativesArray & use_alt_arr, EDAOccurenceArray & use_occ_arr,
                       TDMatrix & use_matrix, EDAOccurencesMask & use_occ_mask)
    : ElementDecoderAC(use_alt_arr)
    , _occArr(&use_occ_arr) , _tdMatrix(&use_matrix), _occMask(&use_occ_mask)
    , _fieldIdx(0)
  { }
  ~ElementDecoderByTDef()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_ELEMENTS_DECODER_BY_TYPEDEF */

