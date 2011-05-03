#ifndef _SCAG_UTIL_STORAGE_DISKFLUSHER_H
#define _SCAG_UTIL_STORAGE_DISKFLUSHER_H

#include <string>
#include <vector>
#include <cassert>
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "FlushConfig.h"
#include "scag/util/Time.h"

namespace scag2 {
namespace util {
namespace storage {

class DiskFlusher : public smsc::core::threads::Thread
{
protected:
    struct DiskFlusherItemBase {
        virtual ~DiskFlusherItemBase() {}
        virtual unsigned flush(util::msectime_type now) = 0;
        virtual void flushIOStatistics( unsigned& pfget,
                                        unsigned& kbget,
                                        unsigned& pfset,
                                        unsigned& kbset ) = 0;
    };

    template < class DiskStorage > class DiskFlusherItem : public DiskFlusherItemBase {
    public:
        DiskFlusherItem( DiskStorage* store ) : store_(store) {}
        virtual unsigned flush( util::msectime_type now ) {
            return store_->flushDirty(now);
        }
        virtual void flushIOStatistics( unsigned& pfget,
                                        unsigned& kbget,
                                        unsigned& pfset,
                                        unsigned& kbset ) {
            return store_->flushIOStatistics(pfget,kbget,pfset,kbset);
        }
    private:
        DiskFlusherItem() : store_(0) {}
    private:
        DiskStorage* store_;
    };

public:
    DiskFlusher( const std::string& name, const FlushConfig& fc ) :
    name_(name),
    log_(smsc::logger::Logger::getInstance(("dflush." + name).c_str())),
    flushConfig_(fc),
    started_(false) {}

    const std::string& getName() const { return name_; }

    ~DiskFlusher();

    void start() 
    {
        if (started_) return;
        smsc::core::synchronization::MutexGuard mg(mon_);
        started_ = true;
        Start();
    }

    void stop() {
        if (!started_) return;
        {
            smsc::core::synchronization::MutexGuard mg(mon_);
            started_ = false;
            mon_.notifyAll();
        }
        WaitFor();
    }

    void wakeup() {
        smsc::core::synchronization::MutexGuard mg(mon_);
        mon_.notify();
    }

    template < class DiskStorage > void add( DiskStorage*       store ) {
        if ( !store ) return;
        smsc::core::synchronization::MutexGuard mg(mon_);
        if ( started_ ) return;
        items_.push_back( new DiskFlusherItem<DiskStorage>(store) );
    }

    virtual int Execute();

    /// flush io statistics and return current increments.
    void flushIOStatistics( unsigned& pfget,
                            unsigned& kbget,
                            unsigned& pfset,
                            unsigned& kbset );

private:
    void clear();

    DiskFlusher();

private:
    std::string                               name_;
    smsc::logger::Logger*                     log_;
    smsc::core::synchronization::EventMonitor mon_;
    std::vector< DiskFlusherItemBase* >       items_; // owned
    FlushConfig                               flushConfig_;
    bool                                      started_;
};

}
}
}

#endif
