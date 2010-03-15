#ifndef _SCAG_UTIL_STORAGE_DISKFLUSHER_H
#define _SCAG_UTIL_STORAGE_DISKFLUSHER_H

#include <string>
#include <vector>
#include <cassert>
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"

namespace scag2 {
namespace util {
namespace storage {

class DiskFlusher : public smsc::core::threads::Thread
{
protected:
    struct DiskFlusherItemBase {
        virtual ~DiskFlusherItemBase() {}
        virtual unsigned flush() = 0;
    };

    template < class DiskStorage > class DiskFlusherItem : public DiskFlusherItemBase {
    public:
        DiskFlusherItem( DiskStorage* store ) : store_(store) {}
        virtual unsigned flush() {
            return store_->flushDirty();
        }
    private:
        DiskFlusherItem() : store_(0) {}
    private:
        DiskStorage* store_;
    };

public:
    DiskFlusher( const std::string& name ) :
    name_(name),
    log_(smsc::logger::Logger::getInstance(name.c_str())),
    maxSpeed_(1000), started_(false) {}


    ~DiskFlusher();

    void start( unsigned maxDirtySpeed ) {
        if (started_) return;
        MutexGuard mg(mon_);
        started_ = true;
        maxSpeed_ = std::max(maxDirtySpeed,10U);
        Start();
    }

    void stop() {
        if (!started_) return;
        {
            MutexGuard mg(mon_);
            started_ = false;
            mon_.notifyAll();
        }
        WaitFor();
    }

    void clear() {
        MutexGuard mg(mon_);
        assert(!started_);
        items_.clear();
    }

    void wakeup() {
        MutexGuard mg(mon_);
        mon_.notify();
    }

    template < class DiskStorage > void add( DiskStorage*       store ) {
        if ( !store ) return;
        MutexGuard mg(mon_);
        if ( started_ ) return;
        items_.push_back( new DiskFlusherItem<DiskStorage>(store) );
    }

    virtual int Execute();

private:
    DiskFlusher();

private:
    std::string                               name_;
    smsc::logger::Logger*                     log_;
    smsc::core::synchronization::EventMonitor mon_;
    std::vector< DiskFlusherItemBase* >       items_; // not owned
    unsigned maxSpeed_;
    bool     started_;
};

}
}
}

#endif
