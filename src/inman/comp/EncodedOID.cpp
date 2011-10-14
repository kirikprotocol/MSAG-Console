#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/comp/EncodedOID.hpp"

namespace smsc {
namespace inman {
namespace comp {

/* ************************************************************************* *
 * Class EncodedOID implementation.
 * ************************************************************************* */
//Splits the 1st subidentifier to 1st and 2nd arcs ids
void EncodedOID::splitIdsPair(const SubIdType & sub_id, SubIdType & id_1st, SubIdType & id_2nd)
{
  if (sub_id < 80) {
    id_1st = sub_id/40; //arc 0 or 1
    id_2nd = sub_id%40;
  } else {              //arc 2
    id_1st = 2;
    id_2nd = sub_id - 80;
  }
}

//assignes value (an encoded OID representation) and optional nickname.
void EncodedOID::init(uint16_t len_octs, const uint8_t * oid_octs, const char * use_nick/* = NULL*/)
{
  mOcts.reserve(len_octs);
  mOcts.clear();
  mOcts.append(oid_octs, len_octs);
  countIds();
  if (use_nick)
    mNick = use_nick;
  else
    mNick = asnValue();
}

//Parses encoded representaion and counts number of SubIds
void EncodedOID::countIds(void)
{
  mNumIds = 0;
  for (uint16_t i = 0; i < length(); ++i) {
    if (!(mOcts[i] & 0x80))
      ++mNumIds;
  }
  if (mNumIds)
    ++mNumIds;  //take in account packing of 1st and 2nd arcs ids
  return;
}

//Decodes subidentifier starting from octet pointed by 'idx'.
//Returns false in case of invalid encoding setting 'sub_id' to (SubIdType)(-1)
bool EncodedOID::parseSubId(SubIdType & sub_id, uint16_t & buf_idx/* >= 0 */) const
{
  for (sub_id = 0; buf_idx < length(); ++buf_idx) {
    uint8_t oct = mOcts[buf_idx];
    SubIdType prev = sub_id;

    sub_id = (sub_id << 7) + (oct & 0x7F);
    if (prev > sub_id)
      break; //SubIdType overloading

    if (!(oct & 0x80)) {
      ++buf_idx;
      return true;
    }
  }
  sub_id = (SubIdType)(-1); //inconsistent encoding
  return false;
}

//Returns OID subidentifier #'id_idx', or (SubIdType)(-1)
//in case of 'id_idx' is out of range or encoding is invalid
//NOTE: 'id_idx' is counted starting from 0
EncodedOID::SubIdType EncodedOID::subId(uint8_t sub_id_idx) const
{
  if (sub_id_idx >= numSubIds())
    return (SubIdType)(-1);

  SubIdType subId = 0;
  uint16_t i = 0;
  if (!parseSubId(subId, i))
    return subId;

  if (sub_id_idx < 2) {
    SubIdType id1st = 0;
    SubIdType id2nd = 0;
    splitIdsPair(subId, id1st, id2nd);
    return (sub_id_idx == 1) ? id1st : id2nd;
  }

  while (--sub_id_idx && parseSubId(subId, i));
  return subId;
}

//Creates string containing ASN.1 Value Notation of this EOID
EncodedOID::ValueString_t EncodedOID::asnValue(void) const
{
  ValueString_t val("");
  if (length()) {
    uint8_t i = 1, numIds = numSubIds();

    sprintf(val.str, "%u", subId(0));
    size_t len = strlen(val.str);
    do {
      snprintf(val.str + len, val.capacity() - len, " %u", subId(i));
      len = strlen(val.str);
    } while ((i < numIds) && (len < val.capacity()));
  }
  return val;
}

bool EncodedOID::operator< (const EncodedOID & cmp_obj) const
{
  if (length() == cmp_obj.length())
    return mOcts.empty() ? false : (memcmp(mOcts.get(), cmp_obj.octets(), length()) < 0);

  if (length() < cmp_obj.length()) {
    int res = memcmp(mOcts.get(), cmp_obj.octets(), length());
    return !res ? true : (res < 0);
  }

  int res = memcmp(mOcts.get(), cmp_obj.octets(), cmp_obj.length());
  return !res ? false : (res < 0);
}


} //comp
} //inman
} //smsc

