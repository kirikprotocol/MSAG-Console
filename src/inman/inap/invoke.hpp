/* ************************************************************************* *
 * Transaction Component: ROS::Invoke
 * ************************************************************************* */
#ifndef __SMSC_INMAN_TCAP_INVOKE__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TCAP_INVOKE__

#include "inman/inap/entity.hpp"
#include "core/buffers/FixedLengthString.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

class Invoke : public TcapEntity {
public:
  enum Status_e {
    resWait = 0, resNotLast, resLast, resError, resLCancel
  };

  enum Response_e {
    respNone = 0, respError, respResultOrError
  };

  static const size_t _idStr_SZ = sizeof("Invoke[%u:%u]") + (sizeof(unsigned)*3)*2;
  typedef smsc::core::buffers::FixedLengthString<_idStr_SZ> IdentString_t;

  static const size_t _statusStr_SZ = sizeof("Invoke[%u:%u]: respType: %u, status: %u") + (sizeof(unsigned)*3)*4;
  typedef smsc::core::buffers::FixedLengthString<_statusStr_SZ> StatusString_t;

  explicit Invoke(uint8_t inv_id = 0, uint8_t op_code = 0, Response_e resp_type = Invoke::respNone)
    : TcapEntity(inv_id, TcapEntity::tceInvoke, op_code)
    , _status(Invoke::resWait), _linkedTo(NULL), _timeout(0), _respType(resp_type)
  { }
  //
  ~Invoke()
  { }

  void        setStatus(Status_e use_st) { _status = use_st; }
  Status_e    getStatus(void) const { return _status; }
  Response_e  getResultType(void) const { return _respType; }
  
  void  linkTo(Invoke * linkedInv) { _linkedTo = linkedInv; }
  const Invoke * getLinkedTo(void) const { return _linkedTo; }

  void setTimeout(uint16_t timeOut) { _timeout = timeOut; }
  uint16_t getTimeout(void) const   { return _timeout; }

  IdentString_t idStr(void) const
  {
    IdentString_t rval;
    snprintf(rval.str, IdentString_t::MAX_SZ - 1,
             "Invoke[%u:%u]", (unsigned)_invId, (unsigned)_opCode);
    return rval;
  }

  StatusString_t strStatus(void) const
  {
    StatusString_t rval;
    snprintf(rval.str, StatusString_t::MAX_SZ - 1,
             "Invoke[%u:%u]: respType: %u, status: %u",
             (unsigned)_invId, (unsigned)_opCode, (unsigned)_respType, (unsigned)_status);
    return rval;
  }

protected:
  Status_e    _status;   //
  Invoke *    _linkedTo; //invoke to which this one linked to
  uint16_t    _timeout;  //response waiting timeout
  Response_e  _respType; //
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCAP_INVOKE__ */

