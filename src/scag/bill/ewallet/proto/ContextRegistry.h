#ifndef SCAG_BILL_EWALLET_PROTO_CONTEXTREGISTRY_H
#define SCAG_BILL_EWALLET_PROTO_CONTEXTREGISTRY_H

#include <memory>
#include <list>
#include "util/int.h"
#include "logger/Logger.h"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/XHash.hpp"
#include "scag/util/XHashPtrFunc.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {


template < class Context > class ContextRegistrySet;

/// registry that keeps the ordered (first in, first out) list of contexts
/// and also a mapping 'seqnum -> context'.
/// NOTE: all methods in this class are not thread-safe
template < class Context > class ContextRegistry 
{

public:
    typedef std::list< Context* >  ContextList;
private:
    typedef smsc::core::buffers::IntHash< typename ContextList::iterator > ContextMap;


public:
    /// a wrapper around an iterator, used to return reference to the hash item.
    class Ctx {
        friend class ContextRegistry;
    public:
        Ctx() : i(0) {}
        Ctx( const Ctx& o ) : i(o.i) { o.i = 0; }
        Ctx& operator = ( Ctx& o ) { if ( &o != this ) {i=o.i; o.i=0;} return *this;}
        Context* getContext() const { return i ? **i : 0; }
        bool operator!() const { return !i;}
        Context* operator -> () { return i ? **i : 0; }
    private:
        Ctx(typename ContextList::iterator* it ) : i(it) {}
    private:
        mutable typename ContextList::iterator *i;
    };


    /// a wrapper around context registry which is used to
    /// provide a locking (thread-safe) access to it.
    class Ptr {
        friend class ContextRegistrySet< Context >;
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

        // ContextRegistry* getRegistry() { return registry_; }

        // void wait() {
        // if ( registry_ ) registry_->mon_.wait(200);
        // }

    private:
        mutable ContextRegistry* registry_;
    };


public:

    bool exists( uint32_t seqNum ) const {
        return map_.Exist(seqNum);
    }

    bool empty() const {
        return list_.empty() && list0_.empty();
    }

    void push( Context* ctx ) {
        typename ContextList::iterator i = 
            ( ctx->getCreationTime() == 0 ?
              list0_.insert(list0_.end(),ctx) :
              list_.insert( list_.end(), ctx) );
        map_.Insert( ctx->getSeqNum(), i );
    }

    Ctx get( uint32_t seqNum ) {
        typename ContextList::iterator* i = map_.GetPtr(seqNum);
        return i;
    }
    
    Context* pop( const Ctx& ptr ) {
        Context* ret = ptr.getContext();
        if ( ret ) {
            map_.Delete(ret->getSeqNum());
            if (ret->getCreationTime()==0) {
                list0_.erase(*ptr.i);
            } else {
                list_.erase(*ptr.i);
            }
        }
        ptr.i = 0;
        return ret;
    }

    /// pop all contexts
    void popAll( ContextList& rv ) {
        rv.clear();
        rv.swap(list_);
        rv.splice(rv.end(), list0_);
        map_.Empty();
    }

    /// pop all expired transactions
    util::msectime_type popExpired( ContextList& rv,
                                    util::msectime_type currentTime,
                                    util::msectime_type timeToSleep )
    {
        rv.clear();
        typename ContextList::iterator i = list_.begin();
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
    ContextList                               list_;
    ContextList                               list0_; // those w/o expiration
    ContextMap                                map_;
    smsc::core::synchronization::EventMonitor mon_;
};



template < class Context > class ContextRegistrySet 
{
public:
    typedef proto::SocketBase* key_type;
    typedef ContextRegistry< Context > Registry;
    typedef typename Registry::Ptr         Ptr;
    typedef typename Registry::Ctx         Ctx;
    typedef typename Registry::ContextList ContextList;
    
private:
    typedef smsc::core::buffers::XHash<key_type,Registry*,util::XHashPtrFunc > HashType;

public:
    ContextRegistrySet() : log_(smsc::logger::Logger::getInstance("ewall.reg")) {}

    ~ContextRegistrySet() {
        ContextList pl;
        while ( true ) {
            {
                smsc::core::synchronization::MutexGuard mg(createMon_);
                typename HashType::Iterator i = set_.getIterator();
                key_type key;
                Registry* reg;
                if ( ! i.Next(key,reg) ) break;
                pl.clear();
                Ptr ptr(reg);
                if ( !ptr->empty() ) ptr->popAll(pl);
                set_.Delete(key);
            }
            Context* c;
            while ( !pl.empty() ) {
                c = pl.front();
                pl.pop_front();
                delete c;
            }
        }
    }


    void create( key_type key ) {
        MutexGuard mg(createMon_);
        if ( ! set_.Exists(key) ) {
            key->attach("ewall.reg");
            Registry* reg = new Registry;
            set_.Insert(key,reg);
            smsc_log_debug(log_,"regptr %p created", reg);
        }
    }


    Ptr get( key_type key ) {
        {
            MutexGuard mg(createMon_);
            Registry** regptr = set_.GetPtr(key);
            if (regptr) return Ptr(*regptr);
        }
        // smsc_log_warn(log_,"context registry for key=%p is not found",key);
        return Ptr();
    }


    void destroy( key_type key ) {
        smsc_log_debug(log_,"destroy %p invoked", key);
        ContextList pl;
        {
            MutexGuard mg(createMon_);
            Registry** regptr = set_.GetPtr(key);
            if ( regptr ) {
                {
                    Ptr ptr(*regptr);
                    if (!ptr->empty()) ptr->popAll(pl);
                }
                smsc_log_debug(log_,"destroying regptr %p", *regptr);
                delete *regptr;
                set_.Delete(key);
                key->detach("ewall.reg");
            }
        }
        if ( !pl.empty() ) {
            smsc_log_warn(log_,"destroying all %u contexts from non-empty registry", unsigned(pl.size()));
            Exception exc("channel is closed", Status::IO_ERROR);
            while ( !pl.empty() ) {
                Context* c = pl.front();
                pl.pop_front();
                c->setError(exc);
                delete c;
            }
        }
    }

private:
    smsc::logger::Logger*                     log_;
    smsc::core::synchronization::EventMonitor createMon_;
    HashType                                  set_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_CONTEXTREGISTRY_H */
