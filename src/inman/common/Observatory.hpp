/* ************************************************************************** *
 * Guarded mutable list of Listeners. Allows safe list modification from
 * Listeners methods.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_GRDOBSERVATORY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
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

        bool isMarked(void) const { return marked ? true : false; }
        bool isFreed(void) const { return freed; }
    };

protected:
    //provides Guard access for GRDObservatoryOfT<> successors
    _GuardArg & Sync(void) { return *(_GuardArg*)this; }

    class GRDNodeList : public std::list<GRDNode> {
    public:
        GRDNodeList() { }
        ~GRDNodeList() { }

        typedef typename GRDNodeList::iterator GRDIterator;

        //Searches list for a node containing given value.
        //Additionally cleans up released nodes.
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
        //Searches list for released nodes starting from the first
        //one up to node pointed by given iterator, and erases nodes
        //which may be safely erased
        void cleanUp(const GRDIterator& up_to)
        {
            GRDIterator it = this->begin();
            while (it != up_to) {
                GRDIterator cit = it++;
                if (cit->isFreed() && !cit->isMarked())
                    this->erase(cit);
            }
        }
        //Searches whole list for released nodes, and erases nodes
        //which may be safely erased
        void cleanAll(void)
        {
            cleanUp(this->end());
        }

        //Releases (marks this node as target for erasing)
        //Returns true if node may be safely erased
        bool releaseNode(GRDIterator & it)
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
        listeners.cleanAll();
        listeners.push_back(GRDNode(use_lstr));
    }
    void removeListener(_GuardedTArg * use_lstr)
    {
        MutexGuard tmp(Sync());
        typename GRDNodeList::GRDIterator it = listeners.findNode(use_lstr);
        if ((it != listeners.end()) && listeners.releaseNode(it))
            listeners.erase(it);
    }
    void folowUp(_GuardedTArg * use_lstr, _GuardedTArg * next_lstr)
    {
        MutexGuard tmp(Sync());
        typename GRDNodeList::GRDIterator it = listeners.findNode(use_lstr);
        if (it != listeners.end())
            ++it;
        listeners.insert(it, GRDNode(next_lstr));
    }

    bool empty(void) const
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
        typename GRDNodeList::GRDIterator it = listeners.findNode(prev->val);
        prev->unmark();
        if ((it == listeners.end()) || (++it) == listeners.end())
            return NULL;
        it->mark();
        return it.operator->();
    }

};

} //util
} //smsc

#endif /* __SMSC_UTIL_GRDOBSERVATORY_HPP */

