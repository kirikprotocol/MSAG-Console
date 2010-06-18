#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include <netinet/in.h>
#include "eyeline/tcap/proto/dec/TDTransactionId.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

const asn1::ASTag
  TDOrigTransactionId::_typeTag(asn1::ASTag::tagApplication, 8);

const asn1::ASTag
  TDDestTransactionId::_typeTag(asn1::ASTag::tagApplication, 9);


//Overriden TypeDecoderAC::decode(): converts OCTSTR to uint32_t
asn1::DECResult
  TransactionIdDecoder::decode(const uint8_t * use_enc, asn1::TSLength max_len)
  /*throw(std::exception)*/
{
  asn1::DECResult rval =
    asn1::ber::DecoderOfOCTSTR_T<uint8_t>::decode(use_enc, max_len);

  if (rval.isOk(_relaxedRule)) {
    if (_octs.size() == 3)
      _octs[3] = 0; //expand up to 4 bytes

    if (_octs.size() == 1) {
      *_dVal = (uint32_t)_octs[0];
    } else if (_octs.size() == 2) {
      uint16_t tmp = *(const uint16_t*)_octs.get();
      *_dVal = (uint32_t)ntohs(tmp);
    } else {
      *_dVal = (uint32_t)ntohl(*(const uint32_t*)_octs.get());
    }
  }
  return rval;
}

}}}}
