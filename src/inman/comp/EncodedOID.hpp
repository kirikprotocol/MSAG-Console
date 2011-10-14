/* ************************************************************************* *
 * Encoded ASN.1 Object Identificator.
 * ************************************************************************* */
#ifndef __INMAN_INAP_COMP_EOID_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_INAP_COMP_EOID_HPP

#include <inttypes.h>
#include "core/buffers/LWArrayTraitsInt.hpp"
#include "core/buffers/LWArrayT.hpp"
#include "core/buffers/FixedLengthString.hpp"

namespace smsc {
namespace inman {
namespace comp {

//
class EncodedOID {
public:
  typedef uint16_t SubIdType;

  static const uint16_t k_dfltSubIdsNum = 16;
  static const uint16_t k_dfltASNValueLen = k_dfltSubIdsNum*sizeof(SubIdType)*3 + 2;
  static const uint16_t k_dfltEncodedLen = 1 + (k_dfltSubIdsNum-1)*3; //first two arcs are packed to single byte

  typedef smsc::core::buffers::FixedLengthString<k_dfltASNValueLen> ValueString_t;
  typedef smsc::core::buffers::FixedLengthString<k_dfltASNValueLen> NickString_t;

private:
  typedef smsc::core::buffers::LWArray_T<
    uint8_t, uint16_t, k_dfltEncodedLen, smsc::core::buffers::LWArrayTraitsPOD_T
  > ValueType;

  uint8_t     	mNumIds;
  ValueType   	mOcts;
  NickString_t  mNick;

protected:
  //Splits the 1st subidentifier to 1st and 2nd arcs ids
  static void splitIdsPair(const SubIdType & sub_id, SubIdType & id_1st, SubIdType & id_2nd);

  //Parses encoded representaion and counts number of SubIds
  void countIds(void);
  //Decodes subidentifier starting from octet pointed by 'idx'.
  //Returns false in case of invalid encoding setting 'sub_id' to -1
  bool parseSubId(SubIdType & sub_id, uint16_t & idx/* >= 0 */) const;

public:
  EncodedOID() : mNumIds(0)
  {
    mNick[0] = 0;
  }
  EncodedOID(uint16_t len_octs, const uint8_t * oid_octs, const char * use_nick = NULL)
  {
    init(len_octs, oid_octs, use_nick);
  }
  //length prefixed bytes array not grater than 255 bytes
  explicit EncodedOID(const uint8_t * eoid_octs, const char * use_nick = NULL)
  {
    init((uint16_t)eoid_octs[0], eoid_octs + 1, use_nick);
  }
  ~EncodedOID()
  { }

  //assignes value (an encoded OID representation) and optional nickname.
  void init(uint16_t len_octs, const uint8_t * oid_octs, const char * use_nick = NULL);

  void clear(void)
  {
    mNumIds = 0;
    mOcts.clear();
    mNick.clear();
  }

  bool empty(void) const { return mOcts.empty(); }

  uint8_t numSubIds(void) const { return mNumIds; }
  NickString_t nick(void) const { return mNick; }

  uint16_t length(void) const { return mOcts.size(); }
  const uint8_t * octets(void) const { return mOcts.get(); }

  //Returns OID subidentifier #'id_idx', or (SubIdType)(-1)
  //in case of 'id_idx' is out of range or encoding is invalid
  //NOTE: 'id_idx' is counted starting from 0
  SubIdType subId(uint8_t sub_id_idx) const;

  //Creates string containing ASN.1 Value Notation of this EOID
  ValueString_t asnValue(void) const;

  bool operator< (const EncodedOID & cmp_obj) const;

  bool operator== (const EncodedOID & cmp_obj) const
  {
    return (length() == cmp_obj.length())
          && !memcmp(mOcts.get(), cmp_obj.octets(), length());
  }
  bool operator!= (const EncodedOID & cmp_obj) const
  {
    return !(*this == cmp_obj);
  }
};

} //comp
} //inman
} //smsc

#endif /* __INMAN_INAP_COMP_EOID_HPP */

