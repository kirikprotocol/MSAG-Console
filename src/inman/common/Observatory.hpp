/* ************************************************************************** *
 * Guarded mutable list of Listeners. Allows safe list modification from
 * Listeners methods.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_GRDOBSERVATORY_HPP
#ident "@(#)$Id$"
#define __SMSC_UTIL_GRDOBSERVATORY_HPP

#include <list>

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;

namespace smsc {
namespace util {

/* GRDObservatoryOfT<> USAGE Samples:
 *
 * class Sample : GRDObservatoryOfT< SampleListenerITF > {
 * public:
 *    
 *     void safeNotify(void)    //simple list traversing
 *     {
 *         for (GRDNode *it = begin(); it; it = next(it))
 *             it->val->anyListenerMethod(this);
 *     }
 *
 *     void safeNotify2(void)    //list traversing with additional break condition
 *     {
 *         GRDNode *it = begin();
 *         for (; it && !AdditionalBreakCondition(); it = next(it))
 *             it->val->anyListenerMethod(this);
 *         if (it)
 *             it->unmark();
 *     }
 * };
 */
template <class _GuardedTArg, class _GuardArg /* : public Mutex */ = Mutex>
class GRDObservatoryOfT : _GuardArg {
public:
    class GRDNode {
    private:
        bool        freed;
        unsigned    marked;

    public:
        _GuardedTArg * val;

        GRDNode(_GuardedTArg * ref_lstr = NULL)
            : freed(false), marked(0), val(ref_lstr)
        { }
        ~GRDNode()
        { }

        void mark(void)   { if (!(++marked)) --marked; } //check for integer overflow
        void unmark(void) { if (marked) --marked; }
        void release(void) { freed = true; }
        bool isMarked(void) { return marked ? true : false; }
        bool isFreed(void) { return freed; }
    };

protected:
    //provides Guard access for GRDObservatoryOfT<> successors
    _GuardArg & Sync(void) { return *(_GuardArg*)this; }

    class GRDNodeList : public std::list<GRDNode> {
    public:
        GRDNodeList() { }
        ~GRDNodeList() { }

        typedef typename GRDNodeList::iterator GRDIterator;

        GRDIterator findNode(const _GuardedTArg * ref_lstr)
        {
            GRDIterator it = this->begin();
            while (it != this->end()) {
                GRDIterator cit = it++;
                if (cit->isFreed() && !cit->isMarked())
                    erase(cit);
                else if (cit->val == ref_lstr)
                    return cit;
            }
            return it;
        }

        void cleanUp(const GRDIterator& up_to)
        {
            GRDIterator it = this->begin();
            while (it != up_to) {
                GRDIterator cit = it++;
                if (cit->isFreed() && !cit->isMarked())
                    erase(cit);
            }
        }

        void cleanAll(void)
        {
            if (!empty()) {
                GRDIterator it = -- this->end();
                it->release();
                cleanUp(this->end());
            }
        }

        inline bool releaseNode(GRDIterator & it)
        {
            it->release();
            return !it->isMarked();
        }
    };

    GRDNodeList listeners;


public:
    GRDObservatoryOfT()
    { }
    virtual ~GRDObservatoryOfT()
    { }

    void addListener(_GuardedTArg * use_lstr)
    {
        MutexGuard tmp(Sync());
        listeners.cleanUp(listeners.end());
        listeners.push_back(GRDNode(use_lstr));
    }
    void removeListener(_GuardedTArg * use_lstr)
    {
        MutexGuard tmp(Sync());
        typename GRDNodeList::GRDIterator it = listeners.findNode(use_lstr);
        if ((it != listeners.end()) && listeners.releaseNode(it))
            listeners.erase(it);
    }
    void folowUp(_GuardedTArg * use_lstr, _GuardedTArg *next_lstr)
    {
        MutexGuard tmp(Sync());
        typename GRDNodeList::GRDIterator it = listeners.findNode(use_lstr);
        if (it != listeners.end()) {
            listeners.insert(++it, GRDNode(next_lstr));
        } else {
            listeners.push_back(GRDNode(next_lstr));
        }
    }

    bool empty(void)
    {
        MutexGuard tmp(Sync());
        return listeners.empty();
    }

    void clearListeners(void)
    {
        MutexGuard tmp(Sync());
        return listeners.clear();
    }

    //increases refCount for returned node if latter exists
    GRDNode * begin(void)
    {
        MutexGuard tmp(Sync());
        typename GRDNodeList::GRDIterator it = listeners.begin();
        if (it != listeners.end()) {
            it->mark();
            return it.operator->();
            
        }
        return NULL;
    }
    //decreases refCount for given node and increases refCount for returned one if it exists
    GRDNode * next(GRDNode * prev)
    {
        MutexGuard tmp(Sync());
        prev->unmark();
        typename GRDNodeList::GRDIterator it = listeners.findNode(prev->val);
        if ((++it) != listeners.end()) {
            it->mark();
            return it.operator->();
        }
        return NULL;
    }

};

} //util
} //smsc

#endif /* __SMSC_UTIL_GRDOBSERVATORY_HPP */

