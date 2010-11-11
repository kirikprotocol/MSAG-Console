#ifndef _SMSC_INFOSME2_SCOREDPTRLIST_H
#define _SMSC_INFOSME2_SCOREDPTRLIST_H

#include <list>
#include <vector>
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
template < class Proc > class ScoredPtrList
{
private:
    typedef typename Proc::ScoredPtrType Ptr;

    struct ScoredPtr {
        Ptr      ptr;
        unsigned score;

        inline ScoredPtr( const Ptr& thePtr, unsigned theScore ) :
        ptr(thePtr), score(theScore) {}

        inline bool operator < ( const ScoredPtr& p ) const {
            if ( score < p.score ) return true;
            return false;
        }

    };

    typedef std::vector< ScoredPtr >          PtrVector;

public:
    typedef typename std::vector< Ptr > PtrList;

    struct isEqual {
        inline isEqual(const Ptr& p) : ptr(p) {}
        inline bool operator () ( const Ptr& p ) const { return p == ptr; }
        const Ptr ptr;
    };

    ScoredPtrList( Proc& proc,
                   unsigned maxdiff,
                   smsc::logger::Logger* logger ) :
    proc_(proc), maxdiff_(maxdiff), log_(logger) {}

    /// process one item from objects.
    /// the ready object is searched and then one item from this object is processed.
    /// if the object is processed successfully, the score is incremented,
    /// otherwise score is also incremented for -increment.
    /// if no object is ready or has been processed then
    /// the time to sleep (time units) is returned (>0), otherwise 0 is returned.
    unsigned processOnce( unsigned deltaTime, unsigned sleepTime )
    {
        unsigned wantToSleep = sleepTime;
        movedObjects_.clear();
        bool needsClean = false;
        try {
            for ( typename PtrVector::iterator i = objects_.begin();
                  i != objects_.end();
                  ++i ) {

                if ( ! i->ptr ) {
                    needsClean = true;
                    continue;
                }

                unsigned objSleep = proc_.scoredObjIsReady( deltaTime, i->ptr );

                if ( objSleep > 0 ) {
                    // object is not ready
                    if ( objSleep < wantToSleep ) wantToSleep = objSleep;
                    continue;
                }
                if (log_ && log_->isDebugEnabled()) {
                    std::string s;
                    proc_.scoredObjToString(s,i->ptr);
                    smsc_log_debug(log_,"obj #%u (%s) score=%u is ready",
                                   unsigned(std::distance(objects_.begin(),i)),
                                   s.c_str(), i->score );
                }

                // object is ready
                const int increment = proc_.processScoredObj( deltaTime, i->ptr );
                i->score += increment < 0 ? unsigned(-increment) : increment;

                if (log_ && log_->isDebugEnabled()) {
                    std::string s;
                    proc_.scoredObjToString(s,i->ptr);
                    smsc_log_debug(log_,"obj #%u (%s) score=%u is %sprocessed, inc=%d",
                                   unsigned(std::distance(objects_.begin(),i)),
                                   s.c_str(), i->score,
                                   increment > 0 ? "" : "NOT ",
                                   increment );
                }

                movedObjects_.push_back(*i);
                i->ptr = Ptr(0);

                if ( increment > 0 ) {
                    // object is processed
                    wantToSleep = 0;
                    break;
                }

                // not processed
            }

        } catch (...) {
            postProcess(needsClean);
            throw;
        }
        postProcess(needsClean);
        return wantToSleep;
    }


    void dump( std::string& s ) const {
        char buf[30];
        for ( typename PtrVector::const_iterator i = objects_.begin();
              i != objects_.end();
              ++i ) {
            if ( !i->ptr ) continue;
            sprintf(buf,"\n %3u. sco=%6u ",
                    unsigned(std::distance(objects_.begin(),i)),
                    i->score );
            s.append(buf);
            proc_.scoredObjToString(s,i->ptr);
        }
    }


    void add( const Ptr& object ) {
        // add an object to the end of the list
        if ( ! object ) return;
        objects_.push_back( ScoredPtr(object, objects_.empty() ? 0 : objects_.back().score ) );
    }


    template < class Pred > void remove( Pred pred, PtrList* res = 0 )
    {
        for ( typename PtrVector::iterator i = objects_.begin();
              i != objects_.end();
              ) {
            if ( pred(i->ptr) ) {
                if (res) res->push_back(i->ptr);
                i = objects_.erase(i);
                continue;
            }
            ++i;
        }
    }

    void clear() {
        objects_.clear();
    }

    inline size_t size() const { return objects_.size(); }

    inline Ptr operator [] ( size_t i ) const {
        const ScoredPtr& o = objects_[i];
        return o.ptr;
    }

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

    void postProcess( bool needsClean ) {
        if ( ! movedObjects_.empty() ) {
            needsClean = true;
        }
        if ( needsClean ) {
            remove(isEqual(Ptr(0)));
        }
        if ( ! movedObjects_.empty() ) {
            std::sort( movedObjects_.begin(), movedObjects_.end() );
            objects_.reserve( objects_.size() + movedObjects_.size() );
            typename PtrVector::iterator middle = objects_.end();
            std::copy( movedObjects_.begin(), movedObjects_.end(),
                       std::back_inserter(objects_) );
            std::inplace_merge( objects_.begin(), middle, objects_.end() );
            bool needfix = false;
            const unsigned diff = objects_.back().score - objects_.front().score;
            const unsigned medi = objects_[objects_.size()/2].score;
            if ( diff > maxdiff_*2 ) {
                needfix = true;
                if (log_) {
                    smsc_log_debug(log_,"too big diff b/w top and bottom: %u, median: %u", diff, medi );
                }
            }
            if ( medi > 1000000 ) {
                needfix = true;
                if (log_) {
                    smsc_log_debug(log_,"too big median: %u", medi );
                }
            }
            if ( needfix ) {
                const unsigned minscore = medi > maxdiff_/2 ? medi - maxdiff_/2 : 0;
                const unsigned maxscore = minscore + maxdiff_;
                for ( typename PtrVector::iterator i = objects_.begin();
                      i != objects_.end(); ++i ) {
                    if ( i->score < minscore ) {
                        i->score = 0;
                    } else if ( i->score > maxscore ) {
                        i->score = maxdiff_;
                    } else {
                        i->score -= minscore;
                    }
                }
            }
        }
    }


private:
    Proc&                 proc_;
    PtrVector             objects_;
    PtrVector             movedObjects_; // temporary vector is here to hold allocated space
    unsigned              maxdiff_;
    smsc::logger::Logger* log_;
};

}
}

#endif
