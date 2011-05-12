#ifndef _SMSC_INFOSME2_SCOREDPTRLIST_H
#define _SMSC_INFOSME2_SCOREDPTRLIST_H

#include <vector>
#include <set>
#include <algorithm>

namespace eyeline {
namespace informer {

struct ScoredPtrListDefaultComp
{
    // comparison is only by score
    template <class T>
        inline bool operator () ( const T& a, const T& b ) {
            return a.score < b.score;
        }
};


/// a templated ScoredPtrList
/// @param Proc is a class that should define the following:
///
/// 1. a typedef for the controlled object
/// typedef ... Proc::ScoredPtrType;  (pointer-like object/a pointer itself)
///
/// 2. a function to check if the object is ready to send
///    @return >0 object is not ready and want to sleep for retval ms,
///            =0 object is ready.
/// unsigned scoredObjIsReady( unsigned deltaTime, Proc::ScoredPtrType& o );
///
/// 3. a function to process the ready object
///    @return >0 object was successfully processed, increment score with retval,
///            <0 object was not processed, increment -retval.
/// int processScoredObj( unsigned deltaTime, Proc::ScoredPtrType& o, unsigned& increment );
///
/// 4. dump obj into string
/// void scoredObjToString( std::string& s, Proc::ScoredPtrType& o );
///
template < class Proc, typename TU = unsigned, class ItemComp = ScoredPtrListDefaultComp > class ScoredPtrList
{
public:
    typedef typename Proc::ScoredPtrType Ptr;
    typedef unsigned ScoreUnit;   // score units
    typedef TU       TimeUnit;    // time units

    struct Item {
        ScoreUnit score;
        Ptr       ptr;
        Item( ScoreUnit s, Ptr p ) : score(s), ptr(p) {}
    };

private:
    typedef std::set< Item, ItemComp > PtrSet;

public:
    typedef typename std::vector< Ptr > PtrList;

    struct isEqual {
        inline isEqual(const Ptr& p) : ptr(p) {}
        inline bool operator () ( const Ptr& p ) const { return p == ptr; }
        const Ptr ptr;
    };

    ScoredPtrList( Proc&        proc,
                   ScoreUnit    maxdiff,
                   smsc::logger::Logger* logger ) :
    proc_(proc), maxdiff_(maxdiff), log_(logger) {}

    /// process one item from objects.
    /// the ready object is searched and then one item from this object is processed.
    /// if the object is processed successfully, the score is incremented,
    /// otherwise score is also incremented for -increment.
    /// @return time to sleep (TimeUnits) until the next object is ready:
    ///  =0 if an object was ready and was procesed;
    ///  >0 if no object is ready / has been processed.
    TimeUnit processOnce( TimeUnit deltaTime, TimeUnit sleepTime )
    {
        TimeUnit wantToSleep = sleepTime;
        // movedObjects_.clear();
        // bool needsClean = false;
        unsigned position = 0;
        PtrSet movedObjects;
        try {
            for ( typename PtrSet::iterator j = objects_.begin();
                  j != objects_.end(); ) {

                typename PtrSet::iterator i = j;
                ++j;
                ++position;

                TimeUnit objSleep = proc_.scoredObjIsReady( deltaTime, i->ptr );

                if ( objSleep > 0 ) {
                    // object is not ready
                    if ( objSleep < wantToSleep ) wantToSleep = objSleep;
                    if (log_ && log_->isDebugEnabled()) {
                        std::string s;
                        proc_.scoredObjToString(s,i->ptr);
                        smsc_log_debug(log_,"obj #%u (%s) score=%u is not ready, objSleep=%u, wantSleep=%u",
                                       position, s.c_str(), unsigned(i->score),
                                       unsigned(objSleep), unsigned(wantToSleep) );
                    }
                    continue;
                }
                if (log_ && log_->isDebugEnabled()) {
                    std::string s;
                    proc_.scoredObjToString(s,i->ptr);
                    smsc_log_debug(log_,"obj #%u (%s) score=%u is ready",
                                   position, s.c_str(), unsigned(i->score) );
                }

                // object is ready
                const int increment = proc_.processScoredObj( deltaTime, i->ptr, objSleep );
                if ( objSleep < wantToSleep ) {
                    // taking actual sleeping time of the object
                    wantToSleep = objSleep;
                }
                ScoreUnit score;
                if ( increment < 0 ) {
                    score = i->score + ScoreUnit(-increment);
                } else if ( increment > 0 ) {
                    score = i->score + ScoreUnit(increment);
                } else {
                    score = i->score + 1;
                }
                
                if (log_ && log_->isDebugEnabled()) {
                    std::string s;
                    proc_.scoredObjToString(s,i->ptr);
                    smsc_log_debug(log_,"obj #%u (%s) score=%u is %sprocessed, inc=%d",
                                   position,
                                   s.c_str(), unsigned(score),
                                   increment > 0 ? "" : "NOT ",
                                   increment );
                }

                movedObjects.insert(Item(score,i->ptr));
                objects_.erase(i);

                if ( increment > 0 ) {
                    // object is processed
                    wantToSleep = 0;
                    break;
                }
                // not processed
            }

        } catch (...) {
            postProcess( movedObjects );
            throw;
        }
        postProcess( movedObjects );
        return wantToSleep;
    }


    void dump( std::string& s ) const {
        char buf[30];
        unsigned position = 0;
        for ( typename PtrSet::const_iterator i = objects_.begin();
              i != objects_.end();
              ++i ) {
            ++position;
            if ( !i->ptr ) continue;
            sprintf(buf,"\n %3u. sco=%6u ",
                    position, unsigned(i->score) );
            s.append(buf);
            proc_.scoredObjToString(s,i->ptr);
        }
    }


    /// NOTE: you may have to ensure that the list does not have an object
    /// via method has() below.
    void add( const Ptr& object ) {
        // add an object to the end of the list
        if ( ! object ) return;
        objects_.insert(Item(objects_.empty() ? 0 :
                             objects_.rbegin()->score,
                             object));
    }


    template < class Pred > bool has( Pred pred )
    {
        for ( typename PtrSet::const_iterator i = objects_.begin();
              i != objects_.end(); ++i ) {
            if ( pred(i->ptr) ) { return true; }
        }
        return false;
    }


    template < class Pred > void remove( Pred pred, PtrList* res = 0 )
    {
        for ( typename PtrSet::iterator i = objects_.begin();
              i != objects_.end();
              ) {
            if ( pred(i->ptr) ) {
                if (res) res->push_back(i->ptr);
                typename PtrSet::iterator j = i;
                ++i;
                objects_.erase(j);
                continue;
            }
            ++i;
        }
    }

    void clear() {
        objects_.clear();
    }

    inline size_t size() const { return objects_.size(); }

protected:

    void postProcess( PtrSet& movedObjects ) {
        if ( movedObjects.empty() ) { return; }
        objects_.insert( movedObjects.begin(), movedObjects.end() );
        bool needfix = false;
        const ScoreUnit a = objects_.begin()->score;
        const ScoreUnit b = objects_.rbegin()->score;
        const ScoreUnit diff = b - a;
        const ScoreUnit medi = (b + a)/2;
        if ( diff > maxdiff_*2 ) {
            needfix = true;
            if (log_) {
                smsc_log_debug(log_,"too big diff b/w top and bottom: %u, median: %u",
                               unsigned(diff), unsigned(medi) );
            }
        }
        if ( medi > 10000000 ) {
            needfix = true;
            if (log_) {
                smsc_log_debug(log_,"too big median: %u", unsigned(medi) );
            }
        }
        if ( needfix ) {
            movedObjects.clear();
            const ScoreUnit minscore = medi > maxdiff_/2 ? medi - maxdiff_/2 : 0;
            const ScoreUnit maxscore = minscore + maxdiff_;
            for ( typename PtrSet::iterator i = objects_.begin();
                  i != objects_.end(); ++i ) {
                ScoreUnit score;
                if ( i->score < minscore ) {
                    score = 0;
                } else if ( i->score > maxscore ) {
                    score = maxdiff_;
                } else {
                    score = i->score - minscore;
                }
                movedObjects.insert(Item(score,i->ptr));
            }
            objects_.swap(movedObjects);
        }
    }


private:
    Proc&                 proc_;
    PtrSet                objects_;
    // PtrSet                movedObjects_; // temporary vector is here to hold allocated space
    ScoreUnit             maxdiff_;
    smsc::logger::Logger* log_;
};

}
}

#endif
