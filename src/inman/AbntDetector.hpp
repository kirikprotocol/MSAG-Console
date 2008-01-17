#pragma ident "$Id$"
/* ************************************************************************** *
 * Billing: implements Billing process logic:
 * 1) determination of abonent contract type by quering abonent provider
 * 2) determination of IN platform and configuring its dialog parameters
 * ************************************************************************** */
#ifndef __SMSC_INMAN_ABNT_DETECTOR_HPP
#define __SMSC_INMAN_ABNT_DETECTOR_HPP

#include "inman/AbntDetManager.hpp"
using smsc::core::timers::TimeWatcherITF;
using smsc::inman::AbonentPolicy;
using smsc::inman::iaprvd::IAPQStatus;
using smsc::inman::iaprvd::IAPQueryListenerITF;
using smsc::inman::iaprvd::AbonentSubscription;

#include "inman/interaction/MsgContract.hpp"
using smsc::inman::interaction::AbntContractReqHandlerITF;
using smsc::inman::interaction::AbntContractRequest;

namespace smsc    {
namespace inman   {

class AbonentDetector : public WorkerAC, public IAPQueryListenerITF,
                     public TimerListenerITF, AbntContractReqHandlerITF {
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
    void handleCommand(INPPacketAC* cmd);
    void Abort(const char * reason = NULL); //aborts billing due to fatal error

    //-- IAPQueryListenerITF interface methods:
    void onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                                                    RCHash qry_status);
    //-- TimerListenerITF interface methods:
    TimeWatcherITF::SignalResult
        onTimerEvent(TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj);

protected:
    bool verifyReq(AbntContractRequest* req);
    //-- AbntContractReqHandlerITF interface methods:
    bool onContractReq(AbntContractRequest* req, uint32_t req_id);

    void doCleanUp(void);
    void ConfigureSCF(void);
    void reportAndExit(void);

    Mutex           _mutex;
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
    AbonentPolicy * abPolicy;
    
    bool StartTimer(void)
    {
        iapTimer.reset(new TimerHdl(_cfg.abtTimeout.CreateTimer(this)));
        if (iapTimer->Id() && (iapTimer->Start() == TimeWatcherITF::errOk)) {
            smsc_log_debug(logger, "%s: started timer[%s]", _logId, iapTimer->IdStr());
            return true;
        }
        smsc_log_error(logger, "%s: failed to start timer[%s]", _logId, iapTimer->IdStr());
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
    inline void SetState(ADState new_state)
    {
        _mutex.Lock();
        _state = new_state;
        _mutex.Unlock();
    }
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_ABNT_DETECTOR_HPP */

