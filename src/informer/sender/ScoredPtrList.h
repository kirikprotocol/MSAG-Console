#ifndef _SMSC_INFOSME2_SCOREDPTRLIST_H
#define _SMSC_INFOSME2_SCOREDPTRLIST_H

#include <vector>
#include <map>
#include <algorithm>

namespace eyeline {
namespace informer {

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
template < class Proc, typename TU = unsigned > class ScoredPtrList
{
public:
    typedef typename Proc::ScoredPtrType Ptr;
    typedef unsigned ScoreUnit;   // score units
    typedef TU       TimeUnit;    // time units

private:
    /*
    struct ScoredPtr {
        Ptr       ptr;
        ScoreUnit score;

        inline ScoredPtr( const Ptr& thePtr, ScoreUnit theScore ) :
        ptr(thePtr), score(theScore) {}

        inline bool operator < ( const ScoredPtr& p ) const {
            if ( score < p.score ) return true;
            return false;
        }
    };
     */

    typedef std::multimap< ScoreUnit, Ptr >  PtrMap;

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
        PtrMap movedObjects;
        try {
            for ( typename PtrMap::iterator j = objects_.begin();
                  j != objects_.end(); ) {

                typename PtrMap::iterator i = j;
                ++j;
                ++position;

                /*
                if ( ! i->second ) {
                    needsClean = true;
                    continue;
                }
                 */

                TimeUnit objSleep = proc_.scoredObjIsReady( deltaTime, i->second );

                if ( objSleep > 0 ) {
                    // object is not ready
                    if ( objSleep < wantToSleep ) wantToSleep = objSleep;
                    if (log_ && log_->isDebugEnabled()) {
                        std::string s;
                        proc_.scoredObjToString(s,i->second);
                        smsc_log_debug(log_,"obj #%u (%s) score=%u is not ready, objSleep=%u, wantSleep=%u",
                                       position, s.c_str(), unsigned(i->first),
                                       unsigned(objSleep), unsigned(wantToSleep) );
                    }
                    continue;
                }
                if (log_ && log_->isDebugEnabled()) {
                    std::string s;
                    proc_.scoredObjToString(s,i->second);
                    smsc_log_debug(log_,"obj #%u (%s) score=%u is ready",
                                   position, s.c_str(), unsigned(i->first) );
                }

                // object is ready
                const int increment = proc_.processScoredObj( deltaTime, i->second, objSleep );
                if ( objSleep < wantToSleep ) {
                    // taking actual sleeping time of the object
                    wantToSleep = objSleep;
                }
                ScoreUnit score;
                if ( increment < 0 ) {
                    score = i->first + ScoreUnit(-increment);
                } else if ( increment > 0 ) {
                    score = i->first + ScoreUnit(increment);
                } else {
                    score = i->first + 1;
                }
                
                if (log_ && log_->isDebugEnabled()) {
                    std::string s;
                    proc_.scoredObjToString(s,i->second);
                    smsc_log_debug(log_,"obj #%u (%s) score=%u is %sprocessed, inc=%d",
                                   position,
                                   s.c_str(), unsigned(score),
                                   increment > 0 ? "" : "NOT ",
                                   increment );
                }

                movedObjects.insert(std::make_pair(score,i->second));
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
        for ( typename PtrMap::const_iterator i = objects_.begin();
              i != objects_.end();
              ++i ) {
            ++position;
            if ( !i->second ) continue;
            sprintf(buf,"\n %3u. sco=%6u ",
                    position, unsigned(i->first) );
            s.append(buf);
            proc_.scoredObjToString(s,i->second);
        }
    }


    /// NOTE: you may have to ensure that the list does not have an object
    /// via method has() below.
    void add( const Ptr& object ) {
        // add an object to the end of the list
        if ( ! object ) return;
        objects_.insert(std::make_pair(objects_.empty() ? 0 :
                                       objects_.rbegin()->first,
                                       object));
    }


    template < class Pred > bool has( Pred pred )
    {
        for ( typename PtrMap::const_iterator i = objects_.begin();
              i != objects_.end(); ++i ) {
            if ( pred(i->second) ) { return true; }
        }
        return false;
    }


    template < class Pred > void remove( Pred pred, PtrList* res = 0 )
    {
        for ( typename PtrMap::iterator i = objects_.begin();
              i != objects_.end();
              ) {
            if ( pred(i->second) ) {
                if (res) res->push_back(i->second);
                typename PtrMap::iterator j = i;
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

    /*
    inline Ptr operator [] ( size_t i ) const {
        const ScoredPtr& o = objects_[i];
        return o.ptr;
    }
     */

    /*
    ScoredPtrList< Proc > copy() const {
        ScoredPtrList< Proc > ret(proc_,maxdiff_,log_);
        ret.objects_ = objects_;
        for ( typename ObjVector::iterator i = ret.objects_.begin();
              i != ret.objects_.end();
              ++i ) {
            i->score = 0;
        }
        return ret;
    }
     */

protected:

    void postProcess( PtrMap& movedObjects ) {
        if ( movedObjects.empty() ) { return; }
        objects_.insert( movedObjects.begin(), movedObjects.end() );
        bool needfix = false;
        const ScoreUnit a = objects_.begin()->first;
        const ScoreUnit b = objects_.rbegin()->first;
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
            for ( typename PtrMap::iterator i = objects_.begin();
                  i != objects_.end(); ++i ) {
                ScoreUnit score;
                if ( i->first < minscore ) {
                    score = 0;
                } else if ( i->first > maxscore ) {
                    score = maxdiff_;
                } else {
                    score = i->first - minscore;
                }
                movedObjects.insert( std::make_pair(score,i->second) );
            }
            objects_.swap(movedObjects);
        }
    }


private:
    Proc&                 proc_;
    PtrMap                objects_;
    // PtrSet                movedObjects_; // temporary vector is here to hold allocated space
    ScoreUnit             maxdiff_;
    smsc::logger::Logger* log_;
};

}
}

#endif
