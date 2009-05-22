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
    _numIds = length() ? 2 : 0;
    for (uint16_t i = 1; i < length(); ++i) {
      if (!(_octs[i] & 0x80))
          ++_numIds;
    }
    return;
  }

  bool getSubId(uint32_t & sub_id, uint16_t & idx/* >= 1 */) const
  {
    for (sub_id = 0; idx < length(); ++idx) {
      uint8_t oct = _octs[idx];
      sub_id = (sub_id << 7) + (oct & 0x7F);
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

  uint32_t  subId(uint16_t idx) const
  {
    if (!idx)
      return (uint32_t)(_octs[0]/40);
    if (idx == 1)
      return (uint32_t)(_octs[0]%40);
    uint32_t subId;
    uint16_t i = 1;
    while (--idx && getSubId(subId, i));
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

