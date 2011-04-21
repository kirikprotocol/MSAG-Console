#ifndef _INFORMER_REGIONSENDER_H
#define _INFORMER_REGIONSENDER_H

#include <map>
#include "informer/data/Region.h"
#include "informer/io/EmbedRefPtr.h"
#include "informer/opstore/RegionalStorage.h"
#include "ScoredPtrList.h"
#include "informer/data/SpeedControl.h"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class SmscSender;

/// NOTE: the synchronization of this class state is done in SmscSender.
class RegionSender
{
    friend class ScoredPtrList< RegionSender >;
    friend class EmbedRefPtr< RegionSender >;

public:
    RegionSender( SmscSender& conn, const RegionPtr& r );

    ~RegionSender() {}

    inline const SmscSender* getConn() const { return conn_; }
    
    // used to switch senders
    // NOTE: this method should be invoked from locked state.
    void assignSender( SmscSender* conn );

    inline regionid_type getRegionId() const {
        return region_.get() ? region_->getRegionId() : 0;
    }

    /// in sms/sec, may be 0
    inline unsigned getBandwidth() const {
        return region_->getBandwidth();
    }

    std::string toString() const;

    inline unsigned isReady(usectime_type currentTime)
    {
        return unsigned(speedControl_.isReady(currentTime % flipTimePeriod, maxSnailDelay));
    }

    /// process region and return the number of seconds to wait until
    /// the region is ready (or 0 if the next attept should be immediate).
    unsigned processRegion(usectime_type currentTime);

    void addDelivery( RegionalStorage& ptr );
    void removeDelivery( dlvid_type dlvId );

private:
    typedef RegionalStorage* ScoredPtrType;

    void scoredObjToString( std::string& s, const ScoredPtrType& dlv )
    {
        if (!dlv) return;
        char buf[20];
        sprintf(buf,"D=%u",dlv->getDlvId());
        s.append(buf);
    }

    unsigned scoredObjIsReady( unsigned unused, ScoredPtrType& dlv );
    int processScoredObj( unsigned unused, ScoredPtrType& dlv, unsigned& objSleep );

    inline void ref() {
        MutexGuard mg(reflock_ MTXWHEREPOST);
        ++ref_;
    }
    inline void unref() {
        {
            MutexGuard mg(reflock_ MTXWHEREPOST);
            if (ref_>1) {
                --ref_;
                return;
            }
        }
        delete this;
    }
    
private:
    typedef std::map< dlvid_type, RegionalStoragePtr > DlvMap;

    smsc::logger::Logger*              log_;

    smsc::core::synchronization::Mutex reflock_;
    unsigned                           ref_;

    smsc::core::synchronization::Mutex lock_;
    SmscSender*                        conn_;     // not owned
    RegionPtr                          region_;   // shared ownership
    DlvMap                             dlvList_;
    ScoredPtrList< RegionSender >      taskList_; // dlvs are not owned

    SpeedControl<usectime_type,tuPerSec> speedControl_; // lock is not needed
    Message                              msg_;    // a cache

    usectime_type                      currentTime_;
    int                                weekTime_;  // local weektime (seconds since monday midnight)
    int                                untilActiveEnd_; // number of seconds until AE
};

typedef EmbedRefPtr< RegionSender > RegionSenderPtr;

} // informer
} // smsc

#endif
