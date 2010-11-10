#ifndef _INFORMER_REGIONSENDER_H
#define _INFORMER_REGIONSENDER_H

#include "informer/data/Region.h"
#include "informer/io/EmbedRefPtr.h"
#include "informer/opstore/RegionalStorage.h"
#include "ScoredList.h"
#include "SpeedControl.h"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class SmscSender;

/// NOTE: the synchronization of this class state is done in SmscSender.
class RegionSender
{
    friend class ScoredList< RegionSender >;
    friend class EmbedRefPtr< RegionSender >;

public:
    RegionSender( SmscSender& conn, const RegionPtr& r );

    ~RegionSender() {}

    inline const SmscSender* getConn() const { return conn_; }
    
    // used to switch senders
    // NOTE: this method should be invoked from locked state.
    void assignSender( SmscSender* conn, const RegionPtr& r );

    inline regionid_type getRegionId() const {
        return region_.get() ? region_->getRegionId() : 0;
    }

    /// in sms/sec
    inline unsigned getBandwidth() const {
        return region_->getBandwidth();
    }

    std::string toString() const;

    inline unsigned isReady(usectime_type currentTime)
    {
        return speedControl_.isReady(unsigned(currentTime % flipTimePeriod));
    }

    inline void suspend(usectime_type currentTime)
    {
        speedControl_.suspend(unsigned(currentTime % flipTimePeriod));
    }

    unsigned processRegion(usectime_type currentTime) 
    {
        smsc_log_debug(log_,"R=%u processing at %llu",getRegionId(),currentTime);
        static const unsigned sleepTime = unsigned(1*tuPerSec);
        currentTime_ = currentTime;
        MutexGuard mg(lock_);
        return taskList_.processOnce(0,sleepTime);
    }

    void addDelivery( RegionalStorage& ptr );
    void removeDelivery( dlvid_type dlvId );

private:
    typedef RegionalStorage ScoredObjType;

    void scoredObjToString( std::string& s, const ScoredObjType& dlv )
    {
        char buf[20];
        sprintf(buf,"D=%u",dlv.getDlvId());
        s.append(buf);
    }

    unsigned scoredObjIsReady( unsigned unused, ScoredObjType& dlv );
    int processScoredObj( unsigned unused, ScoredObjType& dlv );

    inline void ref() {
        MutexGuard mg(reflock_);
        ++ref_;
    }
    inline void unref() {
        {
            MutexGuard mg(reflock_);
            if (ref_>1) {
                --ref_;
                return;
            }
        }
        delete this;
    }
    
private:
    smsc::logger::Logger*              log_;

    smsc::core::synchronization::Mutex reflock_;
    unsigned                           ref_;

    smsc::core::synchronization::Mutex lock_;
    SmscSender*                        conn_;     // not owned
    RegionPtr                          region_;   // shared ownership
    ScoredList< RegionSender >         taskList_; // not owned

    SpeedControl<tuPerSec,uint64_t>    speedControl_;
    Message                            msg_;    // a cache
    usectime_type                      currentTime_;
};

typedef EmbedRefPtr< RegionSender > RegionSenderPtr;

} // informer
} // smsc

#endif
