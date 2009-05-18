/* ************************************************************************** *
 * Abonent Detector: implements abonent contract and gsmSCF parameters
 * determination logic:
 * 1) determination of abonent contract type by quering abonent provider
 * 2) determination of IN platform and configuring its dialog parameters
 * ************************************************************************** */
#ifndef __INMAN_ABNT_DETECTOR_HPP
#ident "@(#)$Id$"
#define __INMAN_ABNT_DETECTOR_HPP

#include "inman/abprov/IAProvider.hpp"
#include "inman/interaction/msgdtcr/MsgContract.hpp"
#include "inman/services/abdtcr/AbntDtcrManager.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::inman::tcpsrv::WorkerAC;
using smsc::core::timers::TimerListenerITF;
using smsc::core::timers::TimeWatcherITF;
using smsc::core::timers::TimerHdl;
using smsc::core::timers::OPAQUE_OBJ;
using smsc::inman::iaprvd::IAPQueryListenerITF;
using smsc::inman::iaprvd::AbonentSubscription;
using smsc::inman::interaction::INPPacketAC;
using smsc::inman::interaction::AbntContractReqHandlerITF;
using smsc::inman::interaction::AbntContractRequest;

class AbonentDetector : public WorkerAC, IAPQueryListenerITF,
                     TimerListenerITF, AbntContractReqHandlerITF {
public:
    typedef enum {
        adIdle = 0,
        adIAPQuering,
        adIAPQueried,
        adTimedOut,
        adCompleted,
        adReported,    // AD -> SMSC : AbntContractResult
        adAborted
    } ADState;

    AbonentDetector(unsigned w_id, AbntDetectorManager * owner, Logger * uselog = NULL);
    virtual ~AbonentDetector();

    static const char * State2Str(ADState st);
    inline const char * State2Str(void) { return State2Str(_state); }

    //-- WorkerAC interface
    void logState(std::string & use_str) const;
    void handleCommand(INPPacketAC* cmd);
    void Abort(const char * reason = NULL); //aborts billing due to fatal error

protected:
    //-- IAPQueryListenerITF interface methods:
    void onIAPQueried(const AbonentId & ab_number,
                      const AbonentSubscription & ab_info, RCHash qry_status);
    //-- TimerListenerITF interface methods:
    TimeWatcherITF::SignalResult
        onTimerEvent(const TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj);

private:
    mutable Mutex   _mutex;
    ADState         _state;
    AbonentDetectorCFG  _cfg;
                    //prefix for logging info
    char            _logId[sizeof("AbntDet[%u:%u]") + sizeof(unsigned int)*3 + 1];

    AbonentRecord   abRec;      //ab_type = abtUnknown
    TonNpiAddress   abNumber;   //calling abonent ISDN number
    const INScfCFG* abScf;      ////corresponding IN-point configuration 
    bool            providerQueried;
    uint32_t        _wErr;
    std::auto_ptr<TimerHdl> iapTimer;   //timer for InAbonentProvider quering

    bool verifyReq(AbntContractRequest* req);
    //-- AbntContractReqHandlerITF interface methods:
    bool onContractReq(AbntContractRequest* req, uint32_t req_id);

    void doCleanUp(void);
    void ConfigureSCF(void);
    void reportAndExit(void);

    bool StartTimer(void)
    {
        iapTimer.reset(new TimerHdl(_cfg.abtTimeout.CreateTimer(this)));
        TimeWatcherITF::Error tErr = TimeWatcherITF::errBadTimer;
        if (iapTimer->Id() && ((tErr = iapTimer->Start()) == TimeWatcherITF::errOk)) {
            smsc_log_debug(logger, "%s: started timer[%s]", _logId, iapTimer->IdStr());
            return true;
        }
        smsc_log_error(logger, "%s: failed to start timer[%s], code: %u",
                       _logId, iapTimer->IdStr(), tErr);
        return false;
    }
    void StopTimer(void)
    {
        if (iapTimer.get()) {
            smsc_log_debug(logger, "%s: releasing timer[%s]", _logId, iapTimer->IdStr());
            iapTimer->Stop();
            iapTimer.reset();
        }
    }
    void SetState(ADState new_state)
    {
        _mutex.Lock();
        _state = new_state;
        _mutex.Unlock();
    }
};

} //abdtcr
} //inman
} //smsc
#endif /* __INMAN_ABNT_DETECTOR_HPP */

