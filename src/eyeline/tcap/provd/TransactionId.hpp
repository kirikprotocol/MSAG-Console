/* ************************************************************************** *
 * Unified transaction identifier class definition.
 * ************************************************************************** */
#ifndef __TCAP_TRANSACTION_ID_HPP
#ident "@(#)$Id$"
#define __TCAP_TRANSACTION_ID_HPP

#include <inttypes.h>
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

//Unified Transaction Identifier: "[LR]ReqNN:RespNN"
class TransactionId {
public:
  enum Origin_e { orgNotAnId = 0, orgLocal, orgRemote };

protected:
  Origin_e  _origin;    //transaction initiating side
  uint32_t  _localId;   //
  uint32_t  _remoteId;  //

public:
  static const unsigned _maxUTIDStringLen = 2 + 2*sizeof(uint32_t)*3;
  typedef smsc::core::buffers::FixedLengthString<_maxUTIDStringLen> UTIDString;

  TransactionId(Origin_e org_type = orgNotAnId, uint32_t lcl_id = 0, uint32_t rmt_id = 0)
    : _origin(org_type), _localId(lcl_id), _remoteId(rmt_id)
  { }
  ~TransactionId()
  { }

  void init(Origin_e org_type, uint32_t lcl_id, uint32_t rmt_id)
  {
    _origin = orgLocal;
    _localId = lcl_id;
    _remoteId = rmt_id;
  }
  void initLocal(uint32_t lcl_id, uint32_t rmt_id = 0)
  {
    _origin = orgLocal;
    _localId = lcl_id;
    _remoteId = rmt_id;
  }
  void initRemote(uint32_t rmt_id, uint32_t  lcl_id = 0)
  {
    _origin = orgRemote;
    _localId = lcl_id;
    _remoteId = rmt_id;
  }
  void bindRemote(uint32_t rmt_id)
  {
    _remoteId = rmt_id;
  }

  Origin_e  getOrigin(void) const { return _origin; }

  //returns local transaction id 
  uint32_t getIdLocal(void) const
  {
    return !_origin ? 0 : _localId;
  }
  //returns remote transaction id 
  uint32_t getIdRemote(void) const
  {
    return !_origin ? 0 : _remoteId;
  }

  //Returns transaction  id at requesting side
  uint32_t  getReqId(void) const
  {
    return (_origin == orgNotAnId) ? 0 :
              ((_origin == orgLocal) ? _localId : _remoteId);
  }
  //Returns transaction  id at responding side
  uint32_t  gerRspId(void) const
  {
    return (_origin == orgNotAnId) ? 0 :
              ((_origin == orgLocal) ? _remoteId : _localId);
  }

  //unified Transaction Id string representation: "[LR]ReqNN:RespNN"
  UTIDString toString(void) const
  {
    UTIDString str;
    if (_origin == orgNotAnId)
      str = "NotAnId";
    else if (_origin == orgLocal)
      snprintf(str.str, _maxUTIDStringLen, "L%u:%u", _localId, _remoteId);
    else
      snprintf(str.str, _maxUTIDStringLen, "R%u:%u", _remoteId, _localId);
    return str;
  }


  bool operator< (const TransactionId & cmp_id) const
  {
    if (_localId == cmp_id._localId) {
      return (_remoteId == cmp_id._remoteId) ? (_origin < cmp_id._origin)
                                            : (_remoteId < cmp_id._remoteId);
    }
    return _localId < cmp_id._localId;
  }

  bool operator== (const TransactionId & cmp_id) const
  {
    return (_localId == cmp_id._localId)
          && (_remoteId == cmp_id._remoteId)
          && (_origin == cmp_id._origin);
  }
};

class RemoteTransactionId : public TransactionId {
public:
  RemoteTransactionId(uint32_t rmt_id, uint32_t lcl_id = 0)
    : TransactionId(orgRemote, lcl_id, rmt_id)
  { }
};

class LocalTransactionId : public TransactionId {
public:
  LocalTransactionId(uint32_t lcl_id, uint32_t rmt_id = 0)
    : TransactionId(orgLocal, lcl_id, rmt_id)
  { }
};

} //provd
} //tcap
} //eyeline

#endif /* __TCAP_TRANSACTION_ID_HPP */

