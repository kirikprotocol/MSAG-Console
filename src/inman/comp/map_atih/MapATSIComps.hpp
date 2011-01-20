/* ************************************************************************* *
 * MAP ATIH AnyTimeSubscriptionInterrogation Components definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPATSI_COMPS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_MAPATSI_COMPS_HPP__

#include "logger/Logger.h"

#include "inman/GsmSCFInfo.hpp"
#include "inman/comp/compdefs.hpp"
#include "inman/comp/MapOpErrors.hpp"
#include "inman/comp/map_atih/RequestedSubscription.hpp"
#include "inman/comp/ODBDefs.hpp"

namespace smsc {
namespace inman {
namespace comp {
namespace atih {

using smsc::logger::Logger;

using smsc::inman::comp::Component;
using smsc::inman::comp::MAPOpErrorId;
using smsc::inman::comp::UnifiedCSI;
using smsc::inman::comp::CSIUid_e;
using smsc::inman::comp::ODBGeneralData;

//MAP anyTimeInfohandling service
struct MAP_ATIH {
  enum OpCode_e {
    anyTimeSubscriptionInterrogation = 62
  };
};

struct ERR_ATSI {
  enum Code_e {
    unknownSubscriber           = MAPOpErrorId::unknownSubscriber,
    bearerServiceNotProvisioned = MAPOpErrorId::bearerServiceNotProvisioned,
    teleserviceNotProvisioned   = MAPOpErrorId::teleserviceNotProvisioned,
    callBarred                  = MAPOpErrorId::callBarred,
    illegalSSOperation          = MAPOpErrorId::illegalSS_Operation,
    ssNotAvailable              = MAPOpErrorId::ss_NotAvailable,
    dataMissing                 = MAPOpErrorId::dataMissing,
    unexpectedDataValue         = MAPOpErrorId::unexpectedDataValue,
    atsi_NotAllowed             = MAPOpErrorId::atsi_NotAllowed,
    informationNotAvailable     = MAPOpErrorId::informationNotAvailable
  };
};

class ATSIArg : public Component, public RequestedSubscription {
public:
  explicit ATSIArg(Logger * use_log = NULL) : RequestedSubscription()
    , subscrImsi(false)
    , compLogger(use_log ? use_log : Logger::getInstance("smsc.inman.comp.ATSIArg"))
  { }
  explicit ATSIArg(const RequestedSubscription & req_info, Logger * use_log = NULL)
    : RequestedSubscription(req_info)
    , subscrImsi(false)
    , compLogger(use_log ? use_log : Logger::getInstance("smsc.inman.comp.ATSIArg"))
  { }

  virtual ~ATSIArg()
  { }

  //sets ISDN address of requesting point
  void setSCFaddress(const char * addr) throw(CustomException);
  void setSCFaddress(const TonNpiAddress& addr) { scfAdr = addr; }

  //sets subscriber identity: IMSI or MSISDN addr
  void setSubscriberId(const char * addr, bool imsi = true) throw(CustomException);
  //set subscriber identity as MSISDN addr
  void setSubscriberId(const TonNpiAddress& addr)
  {
    subscrAdr = addr; subscrImsi = false;
  }

  void encode(std::vector<unsigned char>& buf) const throw(CustomException);

private:
  bool                  subscrImsi; //subscriber is identified by IMSI, not by MSISDN
  TonNpiAddress         subscrAdr;
  TonNpiAddress         scfAdr;
  Logger*               compLogger;
};

class ATSIRes : public Component {
public:
  typedef std::map<CSIUid_e, GsmSCFinfo>  CSIScfsMap;

  explicit ATSIRes(Logger * use_log = NULL)
      : compLogger(use_log ? use_log : Logger::getInstance("smsc.inman.comp.ATSIRes"))
  { }
  virtual ~ATSIRes()
  { }

  const RequestedSubscription & get(void) const { return _csInfo; }

  bool isCSIpresent(CSIUid_e cs_uid) const
  {
    return (_csInfo.hasCSI() && (_csInfo.getCSI() == cs_uid));
  }

  const GsmSCFinfo * getSCFinfo(CSIUid_e cs_uid) const
  {
    CSIScfsMap::const_iterator cit = _scfCsi.find(cs_uid);
    return (cit == _scfCsi.end()) ? NULL : &(cit->second);
  }

  const CSIScfsMap & getCSIScfs(void) const { return _scfCsi; }

  const ODBGeneralData * getODB(void) const { return _odbGD.empty() ? NULL : &_odbGD; }

  virtual void decode(const std::vector<unsigned char>& buf) throw(CustomException);

private:
  Logger *                compLogger;
  RequestedSubscription   _csInfo;
  CSIScfsMap              _scfCsi;
  ODBGeneralData          _odbGD;
};

}//namespace atih
}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_MAPATSI_COMPS_HPP__ */
