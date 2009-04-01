/* ************************************************************************** *
 * Unified transaction identifier class definition.
 * ************************************************************************** */
#ifndef __TCAP_TRANSACTION_ID_HPP
#ident "@(#)$Id$"
#define __TCAP_TRANSACTION_ID_HPP

#include "util/vformat.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

using smsc::util::format;

//Unified transaction identifier: "[LR]ReqNN:RespNN"
class TransactionId {
protected:
  uint32_t    reqId; //transaction  id at requesting side
  uint32_t    rspId; //transaction  id at responding side

public:
  enum Origin { orgNotAnId = 0, orgLocal, orgRemote };

  const Origin   origin; //transaction initiating side

  TransactionId(Origin org_type = orgNotAnId, uint32_t req_id = 0, uint32_t rsp_id = 0)
    : origin(org_type), reqId(req_id), rspId(rsp_id)
  { }
  TransactionId(const TransactionId & use_trid)
    : origin(use_trid.origin), reqId(use_trid.reqId), rspId(use_trid.rspId)
  { }
  ~TransactionId()
  { }

  //unified Transaction Id value: "[LR]ReqNN:RespNN"
  std::string & value(std::string & str) const
  {
    if (origin == orgNotAnId)
      str = "NotAnId";
    else
      format(str, "%c%u:%u", (origin == iniLocal) ? 'L' : 'R', reqId, rspId);
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
      return !origin ? 0 : ((origin == iniLocal) ? reqId : rspId);
  }
  //returns remote transaction id 
  uint32_t remoteId(void) const
  {
      return !origin ? 0 : ((origin != iniLocal) ? reqId : rspId);
  }

  //sets local transaction id 
  void setIdLocal(uint32_t use_id)
  {
      if (origin == iniLocal)
          reqId = use_id;
      else
          rspId = use_id;
  }
  //sets local transaction id
  void setIdRemote(uint32_t use_id)
  {
      if (origin != iniLocal)
          reqId = use_id;
      else
          rspid = use_id;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_TRANSACTION_ID_HPP */

