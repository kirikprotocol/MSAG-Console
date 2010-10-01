#ifndef _INFORMER_REGIONSENDER_H
#define _INFORMER_REGIONSENDER_H

#include "informer/data/Region.h"
#include "informer/opstore/RegionalStorage.h"
#include "ScoredList.h"
#include "SpeedControl.h"

namespace eyeline {
namespace informer {

class SmscSender;

/// NOTE: the synchronization of this class state is done in SmscSender.
class RegionSender
{
    friend class ScoredList< RegionSender >;

public:
    RegionSender( SmscSender& conn, const RegionPtr& r );
    /*
    conn_(&conn), region_(r),
    log_(smsc::logger::Logger::getInstance("regsend")),
    taskList_(*this,BASEINCREMENT*2,smsc::logger::Logger::getInstance("tsklist")) {}
     */

    ~RegionSender() {}

    // used to switch senders
    // NOTE: this method should be invoked from locked state.
    void assignSender( SmscSender& conn, const RegionPtr& r );

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
        // MutexGuard mg(lock_);
        static const unsigned sleepTime = unsigned(1*tuPerSec);
        currentTime_ = currentTime;
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

private:

private:
    smsc::logger::Logger*              log_;
    SmscSender*                        conn_;     // not owned
    RegionPtr                          region_;   // shared ownership
    ScoredList< RegionSender >         taskList_; // not owned
    // smsc::core::synchronization::Mutex lock_;
    SpeedControl<tuPerSec,uint64_t>    speedControl_;
    // RegionalStoragePtr                 ptr_;
    Message                            msg_;    // a cache
    usectime_type                      currentTime_;
};

} // informer
} // smsc

#endif
