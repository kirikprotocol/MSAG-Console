/* ************************************************************************* *
 * Encoded ASN.1 Object Identifier.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_ENCODED_OID_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ABSTRACT_SYNTAX_ENCODED_OID_DEFS

#include "eyeline/asn1/ObjectID.hpp"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace asn1 {

class EncodedOID {
public:
  static const uint16_t _dfltNickLength = _ObjectID_DFLT_SUBIDS_NUM*(sizeof(SubIdType)*3+1);
  static const uint16_t _dfltValueLength = _ObjectID_DFLT_SUBIDS_NUM*sizeof(SubIdType)*3;

  typedef smsc::core::buffers::LWArray_T<
    uint8_t, uint16_t, _dfltValueLength, smsc::core::buffers::LWArrayTraitsPOD_T
  > ValueType;
  typedef smsc::core::buffers::FixedLengthString<_dfltNickLength> NickString;

private:
  uint8_t     _numIds;
  ValueType   _octs;
  NickString  _nick;

protected:
  //Parses encoded representaion and counts number of SubIds
  void countIds(void);

  //Splits the 1st subidentifier to 1st and 2nd arcs ids
  void splitIdsPair(const SubIdType & sub_id, SubIdType & id_1st, SubIdType & id_2nd) const
  {
    if (sub_id < 80) {
      id_1st = sub_id/40; //arc 0 or 1
      id_2nd = sub_id%40;
    } else {              //arc 2
      id_1st = 2;
      id_2nd = sub_id - 80;
    }
  }
  //Decodes subidentifier starting from octet pointed by 'idx'.
  //Returns false in case of invalid encoding setting 'sub_id' to -1
  bool parseSubId(SubIdType & sub_id, uint16_t & idx/* >= 0 */) const;

public:
  EncodedOID() : _numIds(0)
  {
    _nick[0] = 0;
  }
  EncodedOID(uint16_t len_octs, const uint8_t * oid_octs,
                              const char * use_nick = NULL)
  {
    init(len_octs, oid_octs, use_nick);
  }
  //length prefixed bytes array not grater than 255 bytes
  EncodedOID(const uint8_t * eoid_octs, const char * use_nick = NULL)
  {
    init((uint16_t)eoid_octs[0], eoid_octs + 1, use_nick);
  }

  void reset(void)
  {
    _numIds = 0;
    _octs.clear();
    _nick.clear();
  }
  //
  void init(uint16_t len_octs, const uint8_t * oid_octs,
                              const char * use_nick = NULL)
  {
    _octs.reserve(len_octs);
    _octs.clear();
    _octs.append(oid_octs, len_octs);
    countIds();
    if (use_nick)
      _nick = use_nick;
    else
      _nick = asnValue();
  }

  bool empty(void) const { return _octs.size() == 0; }
  uint8_t numSubIds(void) const { return _numIds; }
  const char * nick(void) const { return _nick.c_str(); }

  uint16_t length(void) const { return _octs.size(); }
  const uint8_t * octets(void) const { return &_octs[0]; }

  //Returns OID subidentifier #'id_idx', or (SubIdType)(-1)
  //in case of 'id_idx' is out of range or encoding is invalid
  //NOTE: 'id_idx' is counted starting from 0
  SubIdType subId(uint8_t id_idx) const;

  //Creates string containing ASN.1 Value Notation of this EOID
  NickString asnValue(void) const;

  bool operator< (const EncodedOID & cmp_obj) const
  {
    if (length() == cmp_obj.length())
      return (memcmp(_octs.get(), cmp_obj.octets(), length()) < 0);
    return length() < cmp_obj.length();
  }
  bool operator== (const EncodedOID & cmp_obj) const
  {
    return (length() == cmp_obj.length())
          && !memcmp(_octs.get(), cmp_obj.octets(), length());
  }
  bool operator!= (const EncodedOID & cmp_obj) const
  {
    return !(*this == cmp_obj);
  }
};

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_ENCODED_OID_DEFS */

