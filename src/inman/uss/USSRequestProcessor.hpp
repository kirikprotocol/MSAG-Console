#ifndef __SMSC_INMAN_USS_USSREQUESTPROCESSOR_HPP__
# define __SMSC_INMAN_USS_USSREQUESTPROCESSOR_HPP__ 1

# include <inman/interaction/connect.hpp>
# include <inman/inap/session.hpp>
# include <inman/inap/map_uss/DlgMapUSS.hpp>
# include <inman/comp/map_uss/MapUSSComps.hpp>
//# include <inman/inerrcodes.hpp>

# include <logger/Logger.h>
# include <util/TonNpiAddress.hpp>
# include "ussmessages.hpp"

# include "UssServiceCfg.hpp"
# include <core/synchronization/Mutex.hpp>
# include <inman/uss/USSBalanceConnect.hpp>

namespace smsc {
namespace inman {
namespace uss {

class USSRequestProcessor : public smsc::inman::inap::uss::USSDhandlerITF {
public:
  USSRequestProcessor(smsc::inman::interaction::Connect* conn, const UssService_CFG& cfg, const USSProcSearchCrit& ussProcSearchCrit);
  ~USSRequestProcessor();
  void setDialogId(uint32_t dialogId);
  void handleRequest(const smsc::inman::interaction::USSRequestMessage* requestObject);

  virtual void onMapResult(smsc::inman::comp::uss::MAPUSS2CompAC* arg);
  //dialog finalization/error handling:
  //if ercode != 0, no result has been got from MAP service,
  //NOTE: MAP dialog may be deleted only from this callback !!!
  virtual void onEndMapDlg(RCHash ercode = 0);
private:
  void sendNegativeResponse();

  smsc::inman::interaction::Connect* _conn;
  smsc::logger::Logger*              _logger;
  smsc::inman::inap::TCSessionSR*    _mapSess;
  smsc::inman::inap::uss::MapUSSDlg* _mapDialog;

  UssService_CFG                     _cfg;

  TonNpiAddress _msISDNAddr;
  uint32_t _dialogId;

  smsc::inman::interaction::USSDATA_T _resultUssData;
  std::string _resultUssAsString;
  bool _resultAsLatin1;
  unsigned char _dcs;

  smsc::core::synchronization::Mutex _callbackActivityLock;

  const USSProcSearchCrit _ussProcSearchCrit;
};

}
}
}

#endif
