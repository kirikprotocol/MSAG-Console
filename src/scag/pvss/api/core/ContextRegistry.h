#ifndef _SCAG_PVSS_CORE_CONTEXTREGISTRY_H
#define _SCAG_PVSS_CORE_CONTEXTREGISTRY_H

#include <list>
#include "core/buffers/IntHash.hpp"
#include "core/buffers/XHash.hpp"
#include "scag/util/XHashPtrFunc.h"
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
    ContextRegistry()
#ifdef INTHASH_USAGE_CHECKING
    : map_(SMSCFILELINE)
#endif
    {}

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
            ProcessingList::iterator i;
            if ( map_.Pop(ret->getSeqNum(),i) ) {
                list_.erase(i);
            }
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
        Ptr() : registry_(0) {}
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

        inline bool operator !() const {
            return !registry_;
        }

        inline operator bool () const {
            return registry_;
        }

        ContextRegistry* operator ->() {
            return registry_;
        }
        
        ContextRegistry* getRegistry() { return registry_; }

        void wait() {
            if ( registry_ ) registry_->mon_.wait(200);
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
    ContextRegistrySet() : log_(smsc::logger::Logger::getInstance("pvss.reg")) {}

    ~ContextRegistrySet() {
        ContextRegistry::ProcessingList pl;
        while ( popAny(pl) ) {
            Context* c;
            while ( ! pl.empty() ) {
                c = pl.front();
                pl.pop_front();
                delete c;
            }
        }
    }


    /// create a context registry for key
    void create( key_type key ) {
        MutexGuard mg(createMon_);
        if ( ! set_.Exists(key) ) {
            ContextRegistry* reg = new ContextRegistry;
            set_.Insert(key,reg);
            smsc_log_debug(log_,"regptr %p created", reg);
        }
    }


    /// get context registry (locked)
    Ptr get( key_type key ) {
        {
            MutexGuard mg(createMon_);
            ContextRegistry** regptr = set_.GetPtr(key);
            if (regptr) return Ptr(*regptr);
        }
        smsc_log_warn(log_,"context registry for key=%p is not found", key);
        return Ptr();
    }


    bool popAny( ContextRegistry::ProcessingList& pl )
    {
        ContextRegistry* value;
        {
            MutexGuard mg(createMon_);
            HashType::Iterator i( set_.getIterator() );
            key_type         key;
            if ( ! i.Next(key,value) ) return false;
            pl.clear();
            Ptr ptr(value);
            if (!ptr->empty()) ptr->popAll(pl);
            set_.Delete(key);
        }
        smsc_log_debug(log_,"destroying regptr %p, popping all %d entries",value,pl.size());
        delete value;
        return true;
    }


    // NOTE: method waits until the whole registry is empty.
    // It is used when sources are shutdowned.
    /*
    void waitUntilEmpty()
    {
        key_type key;
        ContextRegistry* value;
        while ( true ) {
            Ptr ptr;
            {
                MutexGuard mg(createMon_);
                for ( HashType::Iterator i(set_.getIterator()); !ptr; ) {
                    if ( ! i.Next(key,value) ) break;
                    Ptr test(value);
                    if ( ! test->empty() ) ptr = test;
                }
            }
            smsc_log_debug(log_,"regptr %p found", ptr.getRegistry());
            if ( !ptr ) break; // no more items
            // wait until registry is empty
            while ( !ptr->empty() ) {
                smsc_log_debug(log_,"regptr %p is not empty yet", ptr.getRegistry());
                ptr.wait();
            }
        }
        smsc_log_debug(log_,"all registries are empty");
    }
     */

    // destroy a context registry for given key
    void destroy( key_type key ) {
        ContextRegistry::ProcessingList pl;
        {
            MutexGuard mg(createMon_);
            ContextRegistry** regptr = set_.GetPtr(key);
            if ( regptr ) {
                {
                    Ptr ptr(*regptr);
                    if (!ptr->empty()) ptr->popAll(pl);
                }
                smsc_log_debug(log_,"destroying regptr %p",*regptr);
                delete *regptr;
                set_.Delete(key);
            }
        }
        if ( !pl.empty() ) {
            smsc_log_warn(log_,"destroying all %d contexts from non-empty context registry", pl.size());
            while ( ! pl.empty() ) {
                Context* c = pl.front();
                delete c;
                pl.pop_front();
            }
        }
    }

private:
    typedef smsc::core::buffers::XHash<key_type,ContextRegistry*,util::XHashPtrFunc> HashType;

private:
    smsc::logger::Logger*                      log_;
    smsc::core::synchronization::EventMonitor  createMon_;
    HashType                                   set_;
};


} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CONTEXTREGISTRY_H */
