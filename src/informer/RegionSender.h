#ifndef _INFORMER_REGIONSENDER_H
#define _INFORMER_REGIONSENDER_H

#include "Region.h"
#include "Delivery.h"
#include "ScoredList.h"
#include "SpeedControl.h"

namespace smsc {
namespace informer {

class SmscSender;

class RegionSender
{
    static const int BASEINCREMENT = 1000;

public:
    RegionSender( SmscSender& conn, Region* r ) :
    conn_(&conn), region_(r),
    log_(smsc::logger::Logger::getInstance("regsend")),
    taskList_(*this,BASEINCREMENT*2,smsc::logger::Logger::getInstance("tsklist")) {}

    ~RegionSender() { delete region_; }

    inline regionid_type getRegionId() const {
        return region_->getRegionId(); 
    }

    inline unsigned getBandwidth() const {
        return region_->getBandwidth();
    }

    std::string toString() const;

    inline unsigned isReady( unsigned deltaus )
    {
        return speedControl_.isReady(deltaus);
    }

    void addDelivery( Delivery* dlv )
    {
        if (dlv) {
            RegionalStoragePtr ptr = dlv->getRegionalStorage(getRegionId());
            if (ptr.get()) {
                MutexGuard mg(lock_);
                taskList_.add(dlv);
            }
        }
    }

    inline void suspend( unsigned deltaus )
    {
        speedControl_.suspend(deltaus);
    }

    unsigned processRegion( msgtime_type currentTime, unsigned deltaus )
    {
        smsc_log_debug(log_,"rgn=%u processing",getRegionId());
        MutexGuard mg(lock_);
        static const unsigned sleepTime = 100000U;
        currentTime_ = currentTime;
        return taskList_.processOnce(deltaus,sleepTime);
    }

private:
    void scoredObjToString( std::string& s, const Delivery& dlv )
    {
        char buf[20];
        sprintf(buf,"dlv=%u",dlv.getDlvId());
        s.append(buf);
    }

    unsigned scoredObjIsReady( unsigned deltaus, Delivery& dlv );
    int processScoredObj( unsigned deltaus, Delivery& dlv );

private:
    typedef Delivery  ScoredObjType;
    friend class ScoredList< RegionSender >;

private:
    SmscSender*                        conn_;
    Region*                            region_;
    smsc::logger::Logger*              log_;
    ScoredList< RegionSender >         taskList_;
    smsc::core::synchronization::Mutex lock_;
    SpeedControl<1000000U,uint64_t>    speedControl_;
    msgtime_type                       currentTime_;
    RegionalStoragePtr                 ptr_;
    Message                            msg_;
};

} // informer
} // smsc

#endif
