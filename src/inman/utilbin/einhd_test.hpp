/* ************************************************************************** *
 * EIN HD SS7 reconnection test
 * ************************************************************************** */
#ifndef _EINHD_RECONNECT_TEST_HPP
#ident "@(#)$Id$"
#define _EINHD_RECONNECT_TEST_HPP

#include <string>
#include <map>

#ifdef USE_PRIVATE_LOGGER
# include "logger/Logger.h"
  using smsc::logger::Logger;
# define LOGGER_CATEGORY "smsc.inman.inap"
#else
# include <stdio.h>
# define Logger          FILE
# define smsc_log_debug(logger, ...) fprintf(logger, "D: "),fprintf(logger, __VA_ARGS__), fprintf(logger, "\n"), fflush(logger)
# define smsc_log_info(logger, ...)  fprintf(logger, "I: "),fprintf(logger, __VA_ARGS__), fprintf(logger, "\n"), fflush(logger)
# define smsc_log_warn(logger, ...)  fprintf(logger, "W: "),fprintf(logger, __VA_ARGS__), fprintf(logger, "\n"), fflush(logger)
# define smsc_log_error(logger, ...) fprintf(logger, "E: "),fprintf(logger, __VA_ARGS__), fprintf(logger, "\n"), fflush(logger)
# define smsc_log_fatal(logger, ...) fprintf(logger, "F: "),fprintf(logger, __VA_ARGS__), fprintf(logger, "\n"), fflush(logger)
#endif /* USE_PRIVATE_LOGGER */

/* function to be called when a connection is broken */
extern "C" unsigned short
  onEINSS7CpConnectBroken(unsigned short from_usrID,
                         unsigned short to_usrID, unsigned char inst_id);



struct SS7UnitInstance {
  enum ConnectStatus {
    uconnIdle = 0, uconnError, uconnAwaited, uconnOk
  };

  uint8_t         instId;     //SS7 communication unit instanceId, [1..255],
  ConnectStatus   connStatus;

  SS7UnitInstance(uint8_t inst_id = 0)
    : instId(inst_id), connStatus(uconnIdle)
  { }

  SS7UnitInstance(const SS7UnitInstance & unit_inst)
    : instId(unit_inst.instId), connStatus(unit_inst.connStatus)
  { }
};

class SS7UnitInstsMap : public std::map<uint8_t /* instId */, SS7UnitInstance> {
public:
  SS7UnitInstance * getInstance(uint8_t inst_id)
  {
    SS7UnitInstsMap::iterator cit = find(inst_id);
    return cit == end() ? 0 : &(cit->second);
  }

  SS7UnitInstance * addInstance(uint8_t inst_id)
  {
    std::pair<iterator, bool> res = insert(value_type(inst_id, SS7UnitInstance(inst_id)));
    return &(res.first->second);
  }

  const SS7UnitInstance * findInstance(uint8_t inst_id) const
  {
    SS7UnitInstsMap::const_iterator cit = find(inst_id);
    return cit == end() ? 0 : &(cit->second);
  }

  bool isStatus(uint8_t inst_id, SS7UnitInstance::ConnectStatus use_status) const
  {
    const SS7UnitInstance * pInst = findInstance(inst_id);
    return (pInst && (pInst->connStatus == use_status));
  }
};

struct TCAPHD_CFG {
  uint8_t     appInstId;      //local application instanceId, [1..255]
  uint8_t     mpUserId;       //CP userId of local message port owner, [1..19]
  uint16_t    maxMsgNum;      //input SS7 messages buffer size, [1..65535]
  SS7UnitInstsMap instIds;    //SS7 communication unit instanceIds, [1..255]
#ifdef EIN_HD
  std::string rcpMgrAdr;      //CSV list of remote CommonParts Managers host:port
  uint8_t     rcpMgrInstId;   //remote CommonParts Manager instanceId, [0..255],
                              //by default: 0
#endif /* EIN_HD */

  TCAPHD_CFG()
    : appInstId(0), mpUserId(0) //, maxMsgNum(0)
#ifdef EIN_HD
    , rcpMgrInstId(0)
#endif /* EIN_HD */
  { }
};

class TCAPConnector {
public:
  enum SS7State_e { ss7None = 0
    , ss7INITED     = 1 //CP static data is initialized
    , ss7REGISTERED = 2 //remote CP manager and message port owner are registered,
                        //SS7 communication facility is initialized
    , ss7OPENED     = 3 //user input message queue is opened
    , ss7CONNECTED  = 4 //user is connected to at least one TCAP BE unit
  };

private:
    volatile SS7State_e  _ss7State;

    TCAPHD_CFG       _cfg;
#ifdef EIN_HD
    char *          _rcpMgrAdr; //remote CP Manager addresses
#endif /* EIN_HD */

    const char *    _logId;     //logging prefix
    Logger *        logger;

protected:
    //Checks for unconnected TCAP BE instances and returns its total number
    unsigned disconnectedUnits(bool * is_all = NULL) const;
    //Connects currently disconnected TCAP BE instances.
    //Returns number of new instances succesfully connected
    unsigned connectUnits(void);
    //Disonnects all TCAP BE instances.
    void disconnectUnits(void);

public:
    TCAPConnector();
    ~TCAPConnector();

    void Init(const TCAPHD_CFG & use_cfg, Logger * use_log = NULL);

    Logger * getLogger(void) const { return logger; }

    //Returns:  (-1) - failed to connect, 0 - already connected, 1 - successfully connected
    int  connectCP(SS7State_e upTo = ss7CONNECTED);
    void disconnectCP(SS7State_e downTo = ss7None);

    //Notifies dispatcher that broken connection is detected, the connection
    //to given TCAP BE instance will be automatically reestablished.
    void    onDisconnect(unsigned short fromUsrID,
                         unsigned short toUsrID, unsigned char inst);
    void    onDisconnect(unsigned char inst);
};


// EIN HD SS7 reconnection test configuration structure
class TST_CFG {
protected:
  unsigned _inited;

public:
  union ParamMask {
    unsigned  val;
    struct ParamBits {
      unsigned ss7_mpUserId : 1;
#ifdef EIN_HD
      unsigned ss7_rcpMgrAdr : 1;
      unsigned ss7_rcpMgrInstId : 1; //has default
#endif /* EIN_HD */
      unsigned ss7_instIds : 1;
      unsigned tst_tgtState : 1;
      unsigned tst_numAttempts : 1; //has default
      unsigned tst_tmoReconn : 1;   //has default
      unsigned tst_maxFaults : 1;   //has default
    } bit;
  } mask;

  static const unsigned _dfltNumAttempts = 40;
  static const unsigned _dfltMaxFaults = 5;
  static const unsigned _dfltTimeout = 600;

  TCAPHD_CFG  ss7;
  TCAPConnector::SS7State_e tgtState;
  unsigned    numAttemts; //number of test runs to execute
  unsigned    maxFaults;  //maximum number of failed reconnect attempts,
                          //that forces the total reconnect
  unsigned    tmoReconn;  //reconnect timeout, units millisecs
              

  TST_CFG()
    : tgtState(TCAPConnector::ss7None), numAttemts(_dfltNumAttempts)
    , maxFaults(_dfltMaxFaults), tmoReconn(_dfltTimeout)
  {
    mask.val = 0;

    //count the 'inited' bitmask in order to avoid endiannes dependency
    union ParamMask tmp;
    tmp.val = 0;
    tmp.bit.ss7_mpUserId = tmp.bit.ss7_instIds = 1;
#ifdef EIN_HD
    tmp.bit.ss7_rcpMgrAdr = 1;
#endif /* EIN_HD */
    tmp.bit.tst_tgtState = 1;
    /**/
    _inited = tmp.val;
  }

  bool isInited(void) const { return (mask.val & _inited) != 0; }
};


#endif /* _EINHD_RECONNECT_TEST_HPP */

