#ifndef _SCAG_PVSS_CORE_CONTEXTREGISTRY_H
#define _SCAG_PVSS_CORE_CONTEXTREGISTRY_H

#include <list>
#include "core/buffers/IntHash.hpp"
#include "core/buffers/XHash.hpp"
#include "scag/util/XHashPtrFunc.h"
#include "core/synchronization/EventMonitor.hpp"
#include "Context.h"
#include "informer/io/EmbedRefPtr.h"
#include "PvssSocketBase.h"

namespace scag2 {
namespace pvss {
namespace core {

class ContextRegistrySet;

class ContextRegistry
{
    friend class eyeline::informer::EmbedRefPtr< ContextRegistry >;

    static smsc::logger::Logger*              log_;

public:
    typedef std::list< ContextPtr >   ProcessingList;

private:
    typedef smsc::core::buffers::IntHash< ProcessingList::iterator > ProcessingMap;

public:
    ContextRegistry( const PvssSockPtr& socket );

    ~ContextRegistry();

    /*
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
     */


    // check if seqnum exists
    /*
    bool exists(uint32_t seqNum) const
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        return map_.Exist(seqNum);
    }
     */

    bool empty() const 
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        return list_.empty();
    }

    /// push to registry, taking ownership
    bool push( Context* ctx );


    /// get pointer to registry item.
    ContextPtr get( uint32_t seqNum ) {
        smsc::core::synchronization::MutexGuard mg(mon_);
        ProcessingList::iterator* i = map_.GetPtr(seqNum);
        return i ? **i : ContextPtr();
    }


    /// pop context (ownership is returned).
    ContextPtr pop( uint32_t seqNum );

    void popAll(ProcessingList& rv) 
    {
//        smsc_log_debug(log_,"popping all ctxs from %p channel %p",this,socket_.get());
        smsc::core::synchronization::MutexGuard mg(mon_);
        rv.clear();
        rv.swap(list_);
        map_.Empty();
        mon_.notify();
    }
    

    /*
    void destroyAll()
    {
        ProcessingList pl;
        {
            smsc::core::synchronization::MutexGuard mg(mon_);
            pl.swap(list_);
            map_.Empty();
            mon_.notify();
        }
    }
     */


    util::msectime_type popExpired(ProcessingList& rv,
                                   util::msectime_type currentTime,
                                   util::msectime_type timeToSleep) {
        rv.clear();
        smsc::core::synchronization::MutexGuard mg(mon_);
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
        return currentTime + timeToSleep;
    }


private:
    void ref() 
    {
        unsigned long long tot;
        {
            smsc::core::synchronization::MutexGuard mg(reflock_);
            if (!ref_) { ++total_; }
            tot = total_;
            ++ref_;
        }
        if ( tot && 0 == (tot % 1000) ) {
            smsc_log_info(log_,"total number of ContextRegistries: %llu",tot);
        }
    }

    void unref()
    {
        unsigned long long tot;
        {
            smsc::core::synchronization::MutexGuard mg(reflock_);
            if ( --ref_ ) return;
            tot = --total_;
        }
        delete this;
        if ( tot && 0 == (tot % 1000) ) {
            smsc_log_info(log_,"total number of ContextRegistries: %llu",tot);
        }
    }

    smsc::core::synchronization::Mutex        reflock_;
    unsigned                                  ref_;
    static unsigned long long                 total_;

private:
    PvssSockPtr                               socket_;
    ProcessingList                            list_;
    ProcessingMap                             map_;
    mutable smsc::core::synchronization::EventMonitor mon_;

/*
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
 */
};


typedef eyeline::informer::EmbedRefPtr< ContextRegistry >  ContextRegistryPtr;


class ContextRegistrySet
{
public:
    typedef PvssSocketBase*                 key_type;
    // typedef ContextRegistry::Ptr         Ptr;

public:
    ContextRegistrySet() : log_(smsc::logger::Logger::getInstance("pvss.reg")) {}

    ~ContextRegistrySet() {
        smsc::core::synchronization::MutexGuard mg(createMon_);
        set_.Empty();
        /*
        do {
            ContextRegistryPtr result = popAny();
            if ( !result ) break;
            // result->destroyAll();
        } while (true);
         */
    }


    /// create a context registry for key
    void create( key_type key ) {
        PvssSockPtr sock(key);
        smsc::core::synchronization::MutexGuard mg(createMon_);
        if ( ! set_.Exists(key) ) {
            ContextRegistryPtr reg(new ContextRegistry(sock));
            set_.Insert(key,reg);
//            smsc_log_debug(log_,"regptr %p created", reg.get());
        }
    }


    /// get context registry (locked)
    ContextRegistryPtr get( key_type key ) {
        {
            smsc::core::synchronization::MutexGuard mg(createMon_);
            ContextRegistryPtr* regptr = set_.GetPtr(key);
            if (regptr) return *regptr;
        }
        smsc_log_warn(log_,"context registry for key=%p is not found", key);
        return ContextRegistryPtr();
    }


    ContextRegistryPtr popAny()
    {
        ContextRegistryPtr result;
        {
            smsc::core::synchronization::MutexGuard mg(createMon_);
            HashType::Iterator i( set_.getIterator() );
            key_type key;
            if (!i.Next(key,result)) return result;
            set_.Delete(key);
        }
//        smsc_log_debug(log_,"regptr %p popped",result.get());
        return result;
    }


    /*
    bool popAny( ContextRegistry::ProcessingList& pl )
    {
        ContextRegistry* value;
        {
            smsc::core::synchronization::MutexGuard mg(createMon_);
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
     */


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
    ContextRegistryPtr pop( key_type key ) {
        ContextRegistryPtr val;
        {
            smsc::core::synchronization::MutexGuard mg(createMon_);
            if ( !set_.Pop(key,val) ) { return val; }
        }
//        smsc_log_debug(log_,"regptr %p popped",val.get());
        return val;
    }

private:
    typedef smsc::core::buffers::XHash<key_type,ContextRegistryPtr,util::XHashPtrFunc> HashType;

private:
    smsc::logger::Logger*                      log_;
    smsc::core::synchronization::EventMonitor  createMon_;
    HashType                                   set_;
};


} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CONTEXTREGISTRY_H */
