/* ************************************************************************** *
 * USSMan service connect: handles USS requests coming from assigned TCP
 * connect in asynchronous mode, manages corresponding USSD dialogs using
 * TCAP layer.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_USSMAN_CONNECT_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
# define __SMSC_INMAN_USSMAN_CONNECT_HPP__ 1

# include <memory>
# include <set>

# include "logger/Logger.h"
# include "inman/interaction/connect.hpp"
# include "inman/interaction/serializer.hpp"
# include "util/Singleton.hpp"
# include "core/synchronization/Mutex.hpp"
# include "inman/uss/UssServiceCfg.hpp"

namespace smsc {
namespace inman {
namespace uss {

using smsc::logger::Logger;
using smsc::inman::interaction::Connect;

class USSRequestProcessor;

class USSProcSearchCrit {
public:
  USSProcSearchCrit(uint8_t ssn, const TonNpiAddress & addr,
                          uint32_t dialog_id, Connect * conn)
    : _ssn(ssn), _addr(addr), _dialogId(dialog_id), _conn(conn)
  { }

  bool operator<(const USSProcSearchCrit& rhs) const {
    if (_ssn != rhs._ssn)
      return _ssn < rhs._ssn;
    else if (_addr != rhs._addr)
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

class USSManConnect : public smsc::inman::interaction::ConnectListenerITF {
public:
  USSManConnect(unsigned conn_id, Logger * logger, const UssService_CFG& cfg);
  ~USSManConnect();

  void onPacketReceived(smsc::inman::interaction::Connect* conn,
                        std::auto_ptr<smsc::inman::interaction::SerializablePacketAC>& recv_cmd);


  void onConnectError(smsc::inman::interaction::Connect* conn, std::auto_ptr<CustomException>& p_exc);
  smsc::inman::interaction::Connect* getRealConnect();

  void markReqProcessorAsCompleted(USSRequestProcessor* ussReqProc);
private:
  Logger *      _logger;
  char          _logId[sizeof("Con[%u]") + sizeof(unsigned int)*3 + 1];
  const UssService_CFG& _cfg;

  typedef std::vector<USSProcSearchCrit>  CreatedSearchCritList_t;
  CreatedSearchCritList_t _searchCritForCreatedReqProcessors;

  typedef std::set<USSRequestProcessor*> active_req_processes_t;
  active_req_processes_t _activeReqProcessors;

  core::synchronization::Mutex _activeReqProcLock;
};


} //uss
} //inman
} //smsc

#endif /* __SMSC_INMAN_USSMAN_CONNECT_HPP__ */
