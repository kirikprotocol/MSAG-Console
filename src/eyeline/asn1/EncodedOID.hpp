/* ************************************************************************* *
 * Encoded ASN.1 Object Identifier.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_OID_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_OID_DEFS__

#include <inttypes.h>
#include <vector>
#include <string>

namespace eyeline {
namespace asn1 {
//
class EncodedOID {
private:
  uint16_t             _numIds;
  std::vector<uint8_t> _octs;
  std::string          _nick;

protected:
  void countIds(void)
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

  //Splits the 1st subidentifier to 1st and 2nd arcs ids
  void splitIdsPair(const uint32_t & sub_id, uint32_t & id_1st, uint32_t & id_2nd) const
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
  bool getSubId(uint32_t & sub_id, uint16_t & idx/* >= 0 */) const
  {
    for (sub_id = 0; idx < length(); ++idx) {
      uint8_t oct = _octs[idx];
      uint32_t prev = sub_id;

      sub_id = (sub_id << 7) + (oct & 0x7F);
      if (prev > sub_id)
        break; //uint32_t overloading

      if (!(oct & 0x80)) {
        ++idx;
        return true;
      }
    }
    sub_id = (uint32_t)(-1); //inconsistent encoding
    return false;
  }

  void init(uint8_t len_octs, const uint8_t * oid_octs,
                              const char * use_nick = NULL)
  {
    _octs.reserve(len_octs);
    _octs.insert(_octs.end(), oid_octs, oid_octs + len_octs);
    countIds();
    if (use_nick)
      _nick = use_nick;
    else
      _nick = asnValue();
  }

public:
  EncodedOID() : _numIds(0)
  { }
  EncodedOID(uint8_t len_octs, const uint8_t * oid_octs,
                              const char * use_nick = NULL)
  {
    init(len_octs, oid_octs, use_nick);
  }
  //length prefixed bytes array
  EncodedOID(const uint8_t * eoid_octs, const char * use_nick = NULL)
  {
    init(eoid_octs[0], eoid_octs + 1, use_nick);
  }

  uint8_t   length(void) const { return (uint8_t)_octs.size(); }
  uint16_t  numSubIds(void) const { return _numIds; }

  const char *    nick(void) const { return _nick.c_str(); }
  const uint8_t * octets(void) const { return &_octs[0]; }

  //Returns OID subidentifier #'id_idx',
  //-1 in case of 'id_idx' is out of range or encoding is invalid
  //NOTE: 'id_idx' is counted starting from 0
  uint32_t  subId(uint16_t id_idx) const
  {
    if (id_idx >= numSubIds())
      return (uint32_t)(-1);

    uint32_t subId = 0;
    uint16_t i = 0;
    if (!getSubId(subId, i))
      return subId;

    if (id_idx < 2) {
      uint32_t id1st = 0;
      uint32_t id2nd = 0;
      splitIdsPair(subId, id1st, id2nd);
      return (id_idx == 1) ? id1st : id2nd;
    }

    while (--id_idx && getSubId(subId, i));
    return subId;
  }

  std::string asnValue() const
  {
    if (!length())
      return "";

    std::string val;
    char buf[3*sizeof(uint32_t)+2];
    uint16_t i = 1, numIds = numSubIds();

    sprintf(buf, "%u", subId(0)); val += buf;
    do {
      sprintf(buf, " %u", subId(i)); val += buf;
    } while (i < numIds);
    return val;
  }

  bool operator< (const EncodedOID & obj2) const
  {
    return _octs < obj2._octs;
  }
  bool operator== (const EncodedOID & obj2) const
  {
    return _octs == obj2._octs;
  }

};

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_OID_DEFS__ */

