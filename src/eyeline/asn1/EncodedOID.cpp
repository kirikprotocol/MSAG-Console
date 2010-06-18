#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/EncodedOID.hpp"

namespace eyeline {
namespace asn1 {

/* ************************************************************************* *
 * Class EncodedOID implementation.
 * ************************************************************************* */
//Parses encoded representaion and counts number of SubIds
void EncodedOID::countIds(void)
{
  _numIds = 0;
  for (uint16_t i = 0; i < length(); ++i) {
    if (!(_octs[i] & 0x80))
      ++_numIds;
  }
  if (_numIds)
    ++_numIds;  //take in account packing of 1st and 2nd arcs ids
  return;
}

//Decodes subidentifier starting from octet pointed by 'idx'.
//Returns false in case of invalid encoding setting 'sub_id' to (SubIdType)(-1)
bool EncodedOID::parseSubId(SubIdType & sub_id, uint16_t & buf_idx/* >= 0 */) const
{
  for (sub_id = 0; buf_idx < length(); ++buf_idx) {
    uint8_t oct = _octs[buf_idx];
    SubIdType prev = sub_id;

    sub_id = (sub_id << 7) + (oct & 0x7F);
    if (prev > sub_id)
      break; //uint32_t overloading

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
SubIdType EncodedOID::subId(uint8_t id_idx) const
{
  if (id_idx >= numSubIds())
    return (SubIdType)(-1);

  SubIdType subId = 0;
  uint16_t i = 0;
  if (!parseSubId(subId, i))
    return subId;

  if (id_idx < 2) {
    SubIdType id1st = 0;
    SubIdType id2nd = 0;
    splitIdsPair(subId, id1st, id2nd);
    return (id_idx == 1) ? id1st : id2nd;
  }

  while (--id_idx && parseSubId(subId, i));
  return subId;
}

//Creates string containing ASN.1 Value Notation of this EOID
EncodedOID::NickString EncodedOID::asnValue(void) const
{
  EncodedOID::NickString val("");
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



} //asn1
} //eyeline

