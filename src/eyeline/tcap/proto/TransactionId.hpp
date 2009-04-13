/* ************************************************************************** *
 * Unified transaction identifier class definition.
 * ************************************************************************** */
#ifndef __TCAP_TRANSACTION_ID_HPP
#ident "@(#)$Id$"
#define __TCAP_TRANSACTION_ID_HPP

#include <inttypes.h>
#include "util/vformat.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

using smsc::util::format;

//Unified transaction identifier: "[LR]ReqNN:RespNN"
class TransactionId {
public:
  enum Origin_e { orgNotAnId = 0, orgLocal, orgRemote };

protected:
  Origin_e  _origin;  //transaction initiating side
  uint32_t  _reqId;   //transaction  id at requesting side
  uint32_t  _rspId;   //transaction  id at responding side

public:
  TransactionId(Origin_e org_type = orgNotAnId, uint32_t req_id = 0, uint32_t rsp_id = 0)
    : _origin(org_type), _reqId(req_id), _rspId(rsp_id)
  { }
  TransactionId(const TransactionId & use_trid)
    : _origin(use_trid._origin), _reqId(use_trid._reqId), _rspId(use_trid._rspId)
  { }
  ~TransactionId()
  { }

  Origin_e  origin(void) const { return _origin; }

  //unified Transaction Id value: "[LR]ReqNN:RespNN"
  std::string & value(std::string & str) const
  {
    if (_origin == orgNotAnId)
      str = "NotAnId";
    else
      format(str, "%c%u:%u", (_origin == orgLocal) ? 'L' : 'R', _reqId, _rspId);
    return str;
  }

  std::string value(void) const
  {
    std::string str;
    return value(str);
  }

  //returns local transaction id 
  uint32_t localId(void) const
  {
      return !_origin ? 0 : ((_origin == orgLocal) ? _reqId : _rspId);
  }
  //returns remote transaction id 
  uint32_t remoteId(void) const
  {
      return !_origin ? 0 : ((_origin != orgLocal) ? _reqId : _rspId);
  }

  //sets local transaction id 
  void setIdLocal(uint32_t use_id)
  {
      if (_origin == orgLocal)
          _reqId = use_id;
      else
          _rspId = use_id;
  }
  //sets local transaction id
  void setIdRemote(uint32_t use_id)
  {
      if (_origin != orgLocal)
          _reqId = use_id;
      else
          _rspId = use_id;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_TRANSACTION_ID_HPP */

