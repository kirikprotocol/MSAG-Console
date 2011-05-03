/* ************************************************************************** *
 * Abonent Detector: implements abonent contract and gsmSCF parameters
 * determination logic:
 * 1) determination of abonent contract type by quering abonent provider
 * 2) determination of IN platform and configuring its dialog parameters
 * ************************************************************************** */
#ifndef __INMAN_ABNT_DETECTOR_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ABNT_DETECTOR_HPP

#include "inman/common/OptionalObjT.hpp"
#include "inman/abprov/IAProvider.hpp"
#include "inman/interaction/msgdtcr/MsgContract.hpp"
#include "inman/services/abdtcr/AbntDtcrManager.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::util::OptionalObj_T;
using smsc::util::RCHash;

using smsc::inman::tcpsrv::WorkerAC;

using smsc::core::timers::TimerListenerITF;
using smsc::core::timers::TimeWatcherITF;
using smsc::core::timers::TimerHdl;
using smsc::core::timers::OPAQUE_OBJ;

using smsc::inman::iaprvd::IAPQueryListenerITF;
using smsc::inman::iaprvd::AbonentId;
using smsc::inman::iaprvd::AbonentSubscription;

using smsc::inman::interaction::INPPacketAC;
using smsc::inman::interaction::AbntContractReqHandlerITF;
using smsc::inman::interaction::AbntContractRequest;

using smsc::inman::iapmgr::IAPRule;
using smsc::inman::iapmgr::IAPPrio_e;
using smsc::inman::iapmgr::INScfCFG;

class AbonentDetector : public WorkerAC, IAPQueryListenerITF,
                     TimerListenerITF, AbntContractReqHandlerITF {
public:
    enum ADState {
        adIdle = 0,
        adIAPQuering,
        adTimedOut,
        adDetermined,
        adCompleted,    // AD -> SMSC : AbntContractResult
        adAborted
    };

    AbonentDetector(unsigned w_id, AbntDetectorManager * owner, Logger * uselog = NULL);
    virtual ~AbonentDetector();

    static const char * State2Str(ADState st);
    inline const char * State2Str(void) { return State2Str(_state); }

    //-- WorkerAC interface
    void logState(std::string & use_str) const;
    void handleCommand(INPPacketAC* cmd);
    void Abort(const char * reason = NULL); //aborts billing due to fatal error

protected:
    // -------------------------------------------------------
    //-- IAPQueryListenerITF interface methods:
    // -------------------------------------------------------
    //Returns false if listener unable to handle query report right now, so
    //requests query to be rereported.
    bool onIAPQueried(const AbonentId & ab_number,
                      const AbonentSubscription & ab_info, RCHash qry_status);
    //-- TimerListenerITF interface methods:
    TimeWatcherITF::SignalResult
        onTimerEvent(const TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj);

private:
    mutable EventMonitor  _sync;
    ADState               _state;
    AbonentDetectorCFG    _cfg;
    //prefix for logging info
    char                  _logId[sizeof("AbntDet[%u:%u]") + sizeof(unsigned int)*3 + 1];

    AbonentSubscription   abCsi;      //
    TonNpiAddress         abNumber;   //calling abonent ISDN number
    IAPRule               _iapRule;   //abonent policy rule
    IAPPrio_e             _curIAPrvd;  //UId of last IAProvider asked
    volatile bool         providerQueried;
    const INScfCFG *      _cfgScf;    //serving gsmSCF(IN-point) configuration
    uint32_t              _wErr;
    OptionalObj_T<TimerHdl> _iapTimer;   //timer for InAbonentProvider quering

    bool verifyReq(AbntContractRequest* req);
    //-- AbntContractReqHandlerITF interface methods:
    bool onContractReq(AbntContractRequest* req, uint32_t req_id);

    bool sendResult(void);
    //Returns true if qyery is started, so execution will continue in another thread.
    bool startIAPQuery(void);
    void doCleanUp(void);
    //Adjusts the MO-SM gsmSCF parameters combining cache/IAProvider CSIs
    //and gsmSCF parameters from config.xml
    void configureMOSM(void);
    void ConfigureSCF(void);
    void reportAndExit(void);

    bool StartTimer(void);
    void StopTimer(void);
};

} //abdtcr
} //inman
} //smsc
#endif /* __INMAN_ABNT_DETECTOR_HPP */

