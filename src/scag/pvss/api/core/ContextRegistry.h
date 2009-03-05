#ifndef _SCAG_PVSS_CORE_CONTEXTREGISTRY_H
#define _SCAG_PVSS_CORE_CONTEXTREGISTRY_H

#include <list>
#include "core/buffers/IntHash.hpp"
#include "core/buffers/XHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "Context.h"

namespace scag2 {
namespace pvss {
namespace core {

class ContextRegistrySet;

class ContextRegistry
{
public:
    typedef std::list<Context*>   ProcessingList;

private:
    typedef smsc::core::buffers::IntHash< ProcessingList::iterator > ProcessingMap;

public:
    class Ctx
    {
        friend class ContextRegistry;
    public:
        Ctx() : i(0) {}
        Ctx( const Ctx& o ) : i(o.i) { o.i = 0; }
        Ctx& operator = ( Ctx& o ) { if ( &o != this ) {i=o.i; o.i=0;} return *this;}
        Context* getContext() const { return i ? **i : 0; }
        bool operator!() const { return !i;}
    private:
        Ctx(ProcessingList::iterator* it) : i(it) {}
    private:
        mutable ProcessingList::iterator* i;
    };


    /// check if seqnum exists
    bool exists(uint32_t seqNum) const
    {
        return map_.Exist(seqNum);
    }

    bool empty() const 
    {
        return list_.empty();
    }

    /// push to registry, taking ownership
    void push(Context* ctx) {
        bool wasempty = list_.empty();
        ProcessingList::iterator i = list_.insert(list_.end(),ctx);
        map_.Insert(ctx->getSeqNum(),i);
        // FIXME: should we notify on previously empty list?
        if (wasempty) mon_.notify();
    }

    /// get pointer to registry item.
    Ctx get( uint32_t seqNum ) {
        ProcessingList::iterator* i = map_.GetPtr(seqNum);
        return i;
    }

    /// pop context (ownership is returned).
    /// NOTE: ptr is invalidated!
    Context* pop( const Ctx& ptr ) {
        Context* ret = ptr.getContext();
        if (ret) {
            map_.Delete(ret->getSeqNum());
            list_.erase(*ptr.i);
        }
        ptr.i = 0;
        if ( list_.empty() ) mon_.notify();
        return ret;
    }


    void popAll(ProcessingList& rv) {
        rv.clear();
        rv.swap(list_);
        map_.Empty();
        mon_.notify();
    }
    

    util::msectime_type popExpired(ProcessingList& rv,
                                   util::msectime_type currentTime,
                                   util::msectime_type timeToSleep) {
        rv.clear();
        ProcessingList::iterator i = list_.begin();
        for ( ;
              i != list_.end();
              ) {
            util::msectime_type expireTime = (*i)->getCreationTime() + timeToSleep;
            if ( currentTime >= expireTime ) {
                int seqNum = (*i)->getSeqNum();
                map_.Delete(seqNum);
                rv.push_back(*i);
                i = list_.erase(i);
            } else {
                return expireTime;
            }
        }
        return currentTime+timeToSleep;
    }


private:
    ProcessingList                            list_;
    ProcessingMap                             map_;
    smsc::core::synchronization::EventMonitor mon_;

public:
    class Ptr 
    {
        friend class ContextRegistrySet;
        Ptr(ContextRegistry* reg) : registry_(reg)
        {
            if (registry_) registry_->mon_.Lock();
        }
    public:
        ~Ptr() {
            if (registry_) registry_->mon_.Unlock();
        }
        Ptr( const Ptr& o ) : registry_(o.registry_) {
            o.registry_ = 0;
        }
        Ptr& operator = ( Ptr& o ) {
            if ( &o != this ) {
                if ( registry_ ) registry_->mon_.Unlock();
                registry_ = o.registry_;
                o.registry_ = 0;
            }
            return *this;
        }

        bool operator !() const {
            return !registry_;
        }

        ContextRegistry* operator ->() {
            return registry_;
        }

        void wait() {
            if ( registry_ ) registry_->mon_.wait();
        }

    private:
        mutable ContextRegistry* registry_;
    };
};


class ContextRegistrySet
{
public:
    typedef smsc::core::network::Socket* key_type;
    typedef ContextRegistry::Ptr         Ptr;

public:
    /// create a context registry for key
    void create( key_type key ) {
        MutexGuard mg(createMon_);
        if ( ! set_.Exists(key) ) {
            set_.Insert(key,new ContextRegistry);
        }
    }

    /// destroy a context registry for given key
    void destroy( key_type key ) {
        MutexGuard mg(createMon_);
        ContextRegistry** regptr = set_.GetPtr(key);
        if ( regptr ) {
            {
                Ptr ptr(*regptr);
                if (!ptr->empty()) {
                    smsc_log_error(log_,"logic error: non-empty context registry is destroyed");
                    abort();
                }
            }
            set_.Delete(key);
        }
    }

    /// get context registry (locked)
    Ptr get( key_type key ) {
        MutexGuard mg(createMon_);
        ContextRegistry** regptr = set_.GetPtr(key);
        if (!regptr) {
            smsc_log_warn(log_,"logic error: no context registry found");
        }
        return Ptr(*regptr);
    }


    void wait(int msec) {
        MutexGuard mg(createMon_);
        createMon_.wait(msec);
    }

private:
    smsc::logger::Logger*                                 log_;
    smsc::core::synchronization::EventMonitor             createMon_;
    smsc::core::buffers::XHash<key_type,ContextRegistry*> set_;
};


} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CONTEXTREGISTRY_H */
