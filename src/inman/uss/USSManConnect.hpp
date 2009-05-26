/* ************************************************************************** *
 * USSMan service connect: handles USS requests coming from assigned TCP
 * connect in asynchronous mode, manages corresponding USSD dialogs using
 * TCAP layer.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_USSMAN_CONNECT_HPP__
#ident "@(#)$Id$"
# define __SMSC_INMAN_USSMAN_CONNECT_HPP__ 1

# include <memory>
# include <set>

//# include "logger/Logger.h"
# include "inman/interaction/connect.hpp"
# include "inman/interaction/serializer.hpp"
# include "util/Singleton.hpp"

# include "inman/uss/UssServiceCfg.hpp"

namespace smsc {
namespace inman {
namespace uss {

using smsc::inman::interaction::Connect;

class USSProcSearchCrit {
public:
  USSProcSearchCrit(uint8_t ssn, const TonNpiAddress & addr,
                          uint32_t dialog_id, Connect * conn)
    : _ssn(ssn), _addr(addr), _dialogId(dialog_id), _conn(conn)
  { }

  bool operator<(const USSProcSearchCrit& rhs) const {
    if (_ssn != rhs._ssn)
      return _ssn < rhs._ssn;
    else if (_addr.toString() != rhs._addr.toString())
      return _addr.toString() < rhs._addr.toString();
    else if (_dialogId != rhs._dialogId)
      return _dialogId < rhs._dialogId;
    else if (_conn != rhs._conn)
      return _conn < rhs._conn;
    else 
      return false;
  }
private:
  unsigned char _ssn;
  TonNpiAddress _addr;
  uint32_t      _dialogId;
  Connect *     _conn;
};

class DuplicateRequestChecker : public smsc::util::Singleton<DuplicateRequestChecker> {
public:
  bool isRequestRegistered(const USSProcSearchCrit& ussProcSearchCrit) const;
  void registerRequest(const USSProcSearchCrit& ussProcSearchCrit);
  void unregisterRequest(const USSProcSearchCrit& ussProcSearchCrit);
private:
  typedef std::set<USSProcSearchCrit> registered_req_t;
  registered_req_t _registeredRequests;
  mutable smsc::core::synchronization::Mutex _lock;
};

//##ModelId=457534DE0050
class USSManConnect : public smsc::inman::interaction::ConnectListenerITF {
public:
  USSManConnect(unsigned conn_id, smsc::logger::Logger* logger, const UssService_CFG& cfg);
  ~USSManConnect();
  //##ModelId=4575350D008E
  void onPacketReceived(smsc::inman::interaction::Connect* conn,
                         std::auto_ptr<smsc::inman::interaction::SerializablePacketAC>& recv_cmd)
    throw(std::exception);

  //##ModelId=45753514006F
  void onConnectError(smsc::inman::interaction::Connect* conn, std::auto_ptr<CustomException>& p_exc);
private:
  smsc::logger::Logger *_logger;
  char          _logId[sizeof("Con[%u]") + sizeof(unsigned int)*3 + 1];
  const UssService_CFG& _cfg;

  typedef std::vector<USSProcSearchCrit>  CreatedSearchCritList_t;
  CreatedSearchCritList_t _searchCritForCreatedReqProcessors;
};


} //uss
} //inman
} //smsc

#endif /* __SMSC_INMAN_USSMAN_CONNECT_HPP__ */
