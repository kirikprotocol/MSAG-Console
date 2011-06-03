#ifndef _INFORMER_ROLLEDLIST_H
#define _INFORMER_ROLLEDLIST_H

#include <list>
#include "logger/Logger.h"
#include "informer/io/InfosmeException.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Condition.hpp"

namespace eyeline {
namespace informer {

/// A container for journaled items.
/// Minimal requirements on T:
/// T {
///   those required by std::list
///   volatile int numberOfLocks;
/// };
///
/// Minimal reqs on C (container):
/// C {
///   iterator  // forward iterator
///   begin()
///   end()     // is constant in all required ops
///   splice( iter, C&, iter );
///   splice( iter, C& );
///   insert( iter, T& );
/// };
template <typename T, class C = std::list<T> > class RolledList
{
private:
    enum {
        usersMask  = 0x3fffffff,
        rolledMask = 0x40000000,
        deleteMask = 0x80000000,
        lockedMask = rolledMask | deleteMask
    };

public:
    typedef std::list<T>                      container_type;
    typedef typename container_type::iterator iterator_type;
    
    RolledList( unsigned conditionCount = 16 ) :
        log_(smsc::logger::Logger::getInstance("rlist")),
        conds_( new smsc::core::synchronization::Condition[conditionCount] ),
        rolling_(container_.end()),
        conditionCount_(conditionCount),
        rollBlockers_(0) {
            smsc_log_debug(log_,"ctor");
        }


    ~RolledList()
    {
        smsc_log_debug(log_,"dtor");
        if (!container_.empty()) {
            std::terminate();
        }
        delete [] conds_;
    }


    /// this method should be used only in dtor to cleanup elements
    container_type& getContainer() {
        return container_;
    }


    /// a helper class which is used to lock item prior to rolling/destruction
    class ItemLock
    {
        friend class RolledList;
    public:
        ItemLock( RolledList& rl ) : rl_(rl), it_(rl_.container_.end()) {}

        ~ItemLock()
        {
            if ( it_ != rl_.container_.end() ) {
                smsc::core::synchronization::MutexGuard mg(rl_.lock_);
                unlock();
            }
        }

        ItemLock( ItemLock& it ) : rl_(it.rl_), it_(it.it_)
        {
            it.it_ = rl_.container_.end(); 
        }

        // lock item for rolling
        bool lock( iterator_type iter )
        {
            smsc::core::synchronization::MutexGuard mg(rl_.lock_);
            return lockUnsync(iter);
        }

        // lock item for removal, and place it into container
        bool pop( iterator_type iter, container_type& holder )
        {
            smsc_log_debug(rl_.log_,"popping %p",&*iter);
            {
                smsc::core::synchronization::MutexGuard mg(rl_.lock_);
                if (!commonLocking(iter)) return false;
                // wait until number of users become 1
                register unsigned nol = iter->numberOfLocks | deleteMask;
                iter->numberOfLocks = nol;
                smsc_log_debug(rl_.log_,"dlocked %p nl=%x",&*iter,nol);
                while ((nol & usersMask) > 1) {
                    smsc_log_debug(rl_.log_,"signalling all %p nl=%x",&*iter,nol);
                    smsc::core::synchronization::Condition& c(rl_.getCnd(iter));
                    c.SignalAll();
                    smsc_log_debug(rl_.log_,"waiting on %p nl=%x",&*iter,nol);
                    c.WaitOn(rl_.lock_);
                    nol = iter->numberOfLocks;
                }
                rl_.moveRollingIterator();
                // pop element
                holder.splice(holder.begin(),rl_.container_,iter);
            }
            smsc_log_debug(rl_.log_,"popped %p",&*iter);
            // detach element from myself
            iter->numberOfLocks = 0;
            it_ = rl_.container_.end();
            return true;
        }


        // the method is useful for creation of new elements into container
        // which are to be rolled after insertion.
        iterator_type createElement( const T& elt )
        {
            smsc::core::synchronization::MutexGuard mg(rl_.lock_);
            if (it_ != rl_.container_.end()) {
                unlock();
            }
            it_ = rl_.container_.insert(rl_.container_.begin(),elt);
            register unsigned nol = rolledMask + 1;
            it_->numberOfLocks = nol;
            smsc_log_debug(rl_.log_,"created %p nl=%x",&*it_,nol);
            return it_;
        }


        inline iterator_type getIter() const {
            return it_;
        }


    private:
        // must be locked
        bool lockUnsync( iterator_type iter )
        {
            smsc_log_debug(rl_.log_,"locking %p",&*iter);
            if (!commonLocking(iter)) return false;
            unsigned nol = iter->numberOfLocks | rolledMask;
            iter->numberOfLocks = nol;
            smsc_log_debug(rl_.log_,"locked %p nl=%x",&*iter,nol);
            // move rolling iterator
            rl_.moveRollingIterator();
            return true;
        }

        // must be locked
        void unlock()
        {
            // unlock previous
            register unsigned nol = it_->numberOfLocks;
            smsc_log_debug(rl_.log_,"unlocking %p nl=%x",&*it_,nol);
            nol = (nol & ~rolledMask) - 1;
            it_->numberOfLocks = nol;
            if (nol) {
                smsc_log_debug(rl_.log_,"signalling %p nl=%x",&*it_,nol);
                rl_.getCnd(it_).Signal();
            }
            it_ = rl_.container_.end();
        }


        // must be locked
        bool commonLocking(iterator_type iter)
        {
            if (it_ != rl_.container_.end()) {
                unlock();
            }
            register unsigned nol = iter->numberOfLocks;
            if (nol & deleteMask) {
                // iter to become invalid
                smsc_log_debug(rl_.log_,"cannot lock %p nl=%x",&*iter,nol);
                return false;
            }
            iter->numberOfLocks = ++nol;
            if (nol & rolledMask) {
                // the item is locked, wait until it gets unlocked
                smsc::core::synchronization::Condition& c(rl_.getCnd(iter));
                while ( nol & rolledMask ) {
                    smsc_log_debug(rl_.log_,"waiting on %p nl=%x",&*iter,nol);
                    c.Signal();
                    c.WaitOn(rl_.lock_);
                    nol = iter->numberOfLocks;
                }
                if (nol & deleteMask) {
                    // iter to become invalid
                    smsc_log_debug(rl_.log_,"cannot lock %p nl=%x",&*iter,nol);
                    iter->numberOfLocks = --nol;
                    if ( (nol & usersMask) > 1 ) {
                        c.SignalAll();
                    }
                    return false;
                }
            }
            // is free to be used
            it_ = iter;
            return true;
        }


    private:
        ItemLock( const ItemLock& it );
        ItemLock& operator = ( const ItemLock& i );

    private:
        RolledList&   rl_;
        iterator_type it_;
    };


    /// A class useful for massive insertion of elements into container.
    /// It prevents to roll journal away while all elements are actually
    /// inserted into container.
    /// The scenario is the following:
    /// 1. create StopRollLock object, blocking rolling;
    /// 2. prepare a container to be inserted, store all elements into journal;
    /// 3. actually insert the container into RolledList;
    /// 4. destroy StopRollLock, allowing rolling again.
    class StopRollLock
    {
    public:
        StopRollLock( RolledList& rl ) : rl_(rl) {
            smsc::core::synchronization::MutexGuard mg(rl_.lock_);
            ++rl_.rollBlockers_;
            smsc_log_debug(rl_.log_,"stoproll ctor block=%u",rl_.rollBlockers_);
        }

        // insert the whole container.
        // all items must be already rolled between
        // the StopRollLock() and insert().
        // NOTE: all items must have numberOfLocks initialized to 0!
        void insert( container_type& from ) {
            smsc_log_debug(rl_.log_,"massive insertion");
            smsc::core::synchronization::MutexGuard mg(rl_.lock_);
            rl_.container_.splice(rl_.container_.begin(),from);
        }

        ~StopRollLock() {
            smsc::core::synchronization::MutexGuard mg(rl_.lock_);
            --rl_.rollBlockers_;
            smsc_log_debug(rl_.log_,"stoproll dtor block=%u",rl_.rollBlockers_);
        }

    private:
        RolledList& rl_;
    };

    // --- rolling

    // start rolling, return locked item
    void startRolling() {
        smsc_log_debug(log_,"start rolling");
        smsc::core::synchronization::MutexGuard mg(lock_);
        if (rollBlockers_>0) {
            smsc_log_debug(log_,"wait on block=%u",rollBlockers_);
            while (rollBlockers_ > 0) {
                lock_.wait(100);
            }
        }
        if ( rolling_ != container_.end() ) {
            throw eyeline::informer::InfosmeException(eyeline::informer::EXC_LOGICERROR,
                                                      "rolling in progress");
        }
        smsc_log_debug(log_,"stop waiting on block=%u",rollBlockers_);
        rolling_ = container_.begin();
        moveRollingIterator();
        smsc_log_debug(log_,"rolling started");
    }

    // advance to the next rolling item
    bool advanceRolling( ItemLock& holder ) {
        smsc::core::synchronization::MutexGuard mg(lock_);
        while ( rolling_ != container_.end() ) {
            smsc_log_debug(log_,"prelocking next rolled item %p",&*rolling_);
            if ( holder.lockUnsync(rolling_) ) {
                return true;
            }
        }
        return false;
    }

private:

    // must be locked
    void moveRollingIterator()
    {
        while ( rolling_ != container_.end() ) {
            unsigned nol = rolling_->numberOfLocks;
            if ( !(nol & lockedMask) ) break;
            iterator_type prev = rolling_;
            ++rolling_;
            smsc_log_debug(log_,"move rolliter %p nl=%x into %p",
                           &*prev, nol, &*rolling_);
        }
        // FIXME: should we splice it_ to the beginning?
    }


    inline smsc::core::synchronization::Condition& getCnd( iterator_type iter ) {
        return conds_[reinterpret_cast<uint64_t>(reinterpret_cast<const void*>(&*iter))/191%conditionCount_];
    }

private:
    // external lock which is used for sync
    smsc::logger::Logger*                     log_;
    smsc::core::synchronization::EventMonitor lock_;
    smsc::core::synchronization::Condition*   conds_;
    container_type                            container_;
    iterator_type                             rolling_;
    unsigned                                  conditionCount_;
    unsigned                                  rollBlockers_;
};

}
}

#endif /* !_INFOMER_ROLLEDLIST_H */
