/* ************************************************************************* *
 * MAP Network Unstructured Supplementary Service v2 Components definition.
 * ************************************************************************* */
#ifndef __SMSC_USSMAN_MAPUSS_COMPS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_MAPUSS_COMPS_HPP__

#include "logger/Logger.h"
#include "util/TonNpiAddress.hpp"

#include "inman/comp/compdefs.hpp"
#include "inman/comp/MapOpErrors.hpp"

#include "ussman/comp/USSDataString.hpp"

namespace smsc {
namespace ussman {
namespace comp {
namespace uss {

using smsc::logger::Logger;
using smsc::util::TonNpiAddress;
using smsc::util::CustomException;

//using smsc::inman::comp::Component;
using smsc::inman::comp::MAPOpErrorId;

using smsc::ussman::comp::USSDataString;

// Alerting categories are defined in GSM 02.07
enum AlertingPattern_e {
  alertingLevel_0 =     0,
  alertingLevel_1 =     1,
  alertingLevel_2 =     2,
  alertingCategory_1 =  4,
  alertingCategory_2 =  5,
  alertingCategory_3 =  6,
  alertingCategory_4 =  7,
  alertingCategory_5 =  8,
  alertingNotSet =      128 //0x80
};


struct MAPUSS_OpCode {
  enum Id_e {
  processUSS_Request = 59
  //  ,USS_Request = 60
  //  ,USS-Notify = 61
  };
};

struct ERR_ProcessUSS_Request {
  enum Id_e {
    callBarred          = MAPOpErrorId::callBarred,
    systemFailure       = MAPOpErrorId::systemFailure,
    dataMissing         = MAPOpErrorId::dataMissing,
    unexpectedDataValue = MAPOpErrorId::unexpectedDataValue,
    unknownAlphabet     = MAPOpErrorId::unknownAlphabet
  };
};


class ProcessUSSRequestArg : public smsc::inman::comp::Component
                           , public USSDataString {
protected:
  //Optional parameters:
  AlertingPattern_e _alrt;    // == alertingNotSet for absence
  TonNpiAddress     _msAdr;   // MS ISDN address, '\0' or "0" for absence
  Logger *          compLogger;

public:
  explicit ProcessUSSRequestArg(Logger * use_log = NULL)
    : _alrt(alertingNotSet), compLogger(use_log ? use_log : Logger::getInstance("smsc.ussman.comp"))
  { }
  explicit ProcessUSSRequestArg(const USSDataString & uss_data, Logger * use_log = NULL)
    : USSDataString(uss_data)
    , _alrt(alertingNotSet), compLogger(use_log ? use_log : Logger::getInstance("smsc.ussman.comp"))
  { }
  virtual ~ProcessUSSRequestArg()
  { }

  //Optional parameters setters:
  void setAlertingPattern(AlertingPattern_e alrt) { _alrt = alrt; }
  void setMSISDNadr(const TonNpiAddress & msadr) { _msAdr = msadr; }
  void setMSISDNadr(const char * adrStr) throw(CustomException);

  //Optional parameters getters
  bool  msISDNadr_present(void) const;
  bool  msAlerting_present(void) const;
  //returns empty TonNpiAddress if msISDN adr absent
  const TonNpiAddress& getMSISDNadr(void) const   { return _msAdr; }
  //returns alertingNotSet if alerting absent
  AlertingPattern_e getAlertingPattern(void) const { return _alrt; }

  // ---------------------------------------------
  // -- Component virtual methods:
  // ---------------------------------------------
  virtual void encode(std::vector<uint8_t>& buf) const throw(CustomException);
  virtual void decode(const std::vector<uint8_t>& buf) throw(CustomException);
};

class ProcessUSSRequestRes : public smsc::inman::comp::Component
                           , public USSDataString {
protected:
  Logger *  compLogger;

public:
  explicit ProcessUSSRequestRes(Logger * use_log = NULL)
    : compLogger(use_log ? use_log : Logger::getInstance("smsc.ussman.comp"))
  { }
  virtual ~ProcessUSSRequestRes()
  { }

  // ---------------------------------------------
  // -- Component virtual methods:
  // ---------------------------------------------
  virtual void encode(std::vector<uint8_t>& buf) const throw(CustomException);
  virtual void decode(const std::vector<uint8_t>& buf) throw(CustomException);
};

} //uss
} //comp
} //ussman
} //smsc

#endif /* __SMSC_USSMAN_MAPUSS_COMPS_HPP__ */

