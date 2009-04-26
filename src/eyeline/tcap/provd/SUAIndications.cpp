#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/SUAIndications.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

/* ************************************************************************* *
 * class SUAUnitdataInd implementation
 * ************************************************************************* */
//parses MessageInfo::msgData buffer, sets references to MessageInfo::msgData buffer
bool SUAUnitdataInd::_parseMsgInfo(void)
{
  uint32_t bufSz = (uint32_t)_suaMsg->msgData.getDataSize();
  if (bufSz <= (sizeof(uint32_t) + sizeof(uint32_t) +1 +1 +1 + sizeof(uint16_t)))
    return false;

  union { //force correct alignment of uint32_t on 64-bit platforms
      uint16_t    ui16;
      uint32_t    ui32;
      uint8_t     buf[sizeof(uint32_t)];
  } num;

  //msgLen field: 
  uint16_t pos = (uint16_t)_suaMsg->msgData.Copy(num.buf, 0, (unsigned)sizeof(uint32_t));
  uint32_t msgLen = num.ui32;
  //msgType field: just skip 
  pos += (uint16_t)sizeof(uint32_t); 
  //_fieldsMask field
  pos += (uint16_t)_suaMsg->msgData.Copy(&_fieldsMask, pos, 1);
  //_sequenceControl field: optional
  if (_fieldsMask & SUAUnitdataInd::has_SEQUENCE_CONTROL) {
    pos += (uint16_t)_suaMsg->msgData.Copy(num.buf, pos, (unsigned)sizeof(uint32_t));
    _sequenceControl = num.ui32;
  } else
    _sequenceControl = 0;

  //called address
  if ((pos + 1) > bufSz)
    return false;
  pos += (uint16_t)_suaMsg->msgData.Copy(&_calledAddrLen, pos, 1);

  if ((pos + _calledAddrLen) > bufSz)
    return false;
  _calledAddr = _suaMsg->msgData.get() + pos;
  pos += _calledAddrLen;

  //called address
  if ((pos + 1) > bufSz)
    return false;
  pos += (uint16_t)_suaMsg->msgData.Copy(&_callingAddrLen, pos, 1);

  if ((pos + _callingAddrLen) > bufSz)
    return false;
  _callingAddr = _suaMsg->msgData.get() + pos;
  pos += _callingAddrLen;

  //userDatalen field:
  if ((pos + sizeof(uint16_t)) > bufSz)
    return false;
  pos += (uint16_t)_suaMsg->msgData.Copy(num.buf, pos, (unsigned)sizeof(uint16_t));
  _userDataLen = num.ui16;

  //userData field:
  if ((pos + _userDataLen) > bufSz)
    return false;
  _userData = _suaMsg->msgData.get() + pos;

  return true;
}

/* ************************************************************************* *
 * class SUANoticeInd implementation
 * ************************************************************************* */
//parses MessageInfo::msgData buffer, sets references to MessageInfo::msgData buffer
bool SUANoticeInd::_parseMsgInfo(void)
{
  uint32_t bufSz = (uint32_t)_suaMsg->msgData.getDataSize();
  if (bufSz <= (sizeof(uint32_t) + sizeof(uint32_t) +1 +1 +1 + sizeof(uint16_t) + 1))
    return false;

  union { //force correct alignment of buf storing uint32_t on 64-bit platforms
      uint16_t    ui16;
      uint32_t    ui32;
      uint8_t     buf[sizeof(uint32_t)];
  } num;

  //msgLen field: 
  uint16_t pos = (uint16_t)_suaMsg->msgData.Copy(num.buf, 0, (unsigned)sizeof(uint32_t));
  uint32_t msgLen = num.ui32;
  //msgType field: just skip 
  pos += (uint16_t)sizeof(uint32_t); 
  //_fieldsMask field
  pos += (uint16_t)_suaMsg->msgData.Copy(&_fieldsMask, pos, 1);

  //called address
  if ((pos + 1) > bufSz)
    return false;
  pos += (uint16_t)_suaMsg->msgData.Copy(&_calledAddrLen, pos, 1);

  if ((pos + _calledAddrLen) > bufSz)
    return false;
  _calledAddr = _suaMsg->msgData.get() + pos;
  pos += _calledAddrLen;

  //calling address
  if ((pos + 1) > bufSz)
    return false;
  pos += (uint16_t)_suaMsg->msgData.Copy(&_callingAddrLen, pos, 1);

  if ((pos + _callingAddrLen) > bufSz)
    return false;
  _callingAddr = _suaMsg->msgData.get() + pos;
  pos += _callingAddrLen;

  //return reason field
  pos += (uint16_t)_suaMsg->msgData.Copy(&_returnReason, pos, 1);

  //userDatalen field:
  if ((pos + sizeof(uint16_t)) > bufSz)
    return false;
  pos += (uint16_t)_suaMsg->msgData.Copy(num.buf, pos, (unsigned)sizeof(uint16_t));
  _userDataLen = num.ui16;

  //userData field:
  if ((pos + _userDataLen) > bufSz)
    return false;
  _userData = _suaMsg->msgData.get() + pos;
  pos += _userDataLen;

  //_importance field: optional
  if (_fieldsMask & SUANoticeInd::has_IMPORTANCE) {
    pos += (uint16_t)_suaMsg->msgData.Copy(&_importance, pos, 1);
//    _importance &= 0x07;
  } else
    _importance = 0;

  return true;
}

} //provd
} //tcap
} //eyeline

