/* ************************************************************************** *
 *
 * ************************************************************************** */
#ifndef __SMSC_INMAN_USS_USSREQUESTPROCESSOR_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
# define __SMSC_INMAN_USS_USSREQUESTPROCESSOR_HPP__ 1

//# include "logger/Logger.h"
//# include "util/TonNpiAddress.hpp"
//# include "core/synchronization/Mutex.hpp"
# include "inman/interaction/connect.hpp"
# include "inman/inap/HDSSnSession.hpp"
# include "inman/inap/map_uss/DlgMapUSS.hpp"
# include "inman/comp/map_uss/MapUSSComps.hpp"

# include "inman/uss/UssServiceCfg.hpp"
# include "inman/uss/ussmessages.hpp"
# include "inman/uss/USSManConnect.hpp"

namespace smsc {
namespace inman {
namespace uss {

using smsc::core::synchronization::Mutex;
using smsc::inman::interaction::USSDATA_T;
using smsc::inman::inap::uss::MapUSSDlg;


class USSRequestProcessor : public smsc::inman::inap::uss::USSDhandlerITF {
public:
  USSRequestProcessor(USSManConnect* ussManConn, Connect * conn, const UssService_CFG & cfg,
                      uint32_t dialog_id, const USSProcSearchCrit & ussProcSearchCrit,
                      Logger * use_log = NULL);
  ~USSRequestProcessor();

  void handleRequest(const smsc::inman::interaction::USSRequestMessage * requestObject);

  virtual void onMapResult(smsc::inman::comp::uss::MAPUSS2CompAC* arg);
  //dialog finalization/error handling:
  //if ercode != 0, no result has been got from MAP service,
  //NOTE: MAP dialog may be deleted only from this callback !!!
  virtual void onEndMapDlg(RCHash ercode = 0);

  void markConnectAsClosed();
private:
  void sendNegativeResponse();
  TCSessionSR * getMAPSession(uint8_t rmt_ssn, const TonNpiAddress & rnpi);
  void sendPacket(inman::interaction::SPckUSSResult* resultPacket);
  void finalizeRequest();

  bool _isRunning;
  USSManConnect*            _ussManConn;
  Connect *                 _conn;
  const UssService_CFG &    _cfg;
  uint32_t                  _dialogId;
  const USSProcSearchCrit   _ussProcSearchCrit;
  Logger *                  _logger;

  char          _logId[sizeof("USSReq[%u:%u]") + sizeof(unsigned int)*3 + 1];
  MapUSSDlg *   _mapDialog;
  TonNpiAddress _msISDNAddr;
  USSDATA_T     _resultUssData;
  std::string   _resultUssAsString;
  bool          _resultAsLatin1;
  unsigned char _dcs;
  core::synchronization::Mutex _statusLock, _callbackActivityLock, _connLock;
};

} //uss
} //inman
} //smsc

#endif /* __SMSC_INMAN_USS_USSREQUESTPROCESSOR_HPP__ */

