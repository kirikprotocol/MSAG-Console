#ifndef _SCAG_PROTOTYPES_INFOSME_SCOREDLIST_H
#define _SCAG_PROTOTYPES_INFOSME_SCOREDLIST_H

#include <vector>
#include <algorithm>

namespace scag2 {
namespace prototypes {
namespace infosme {

/// a templated ScoredList
/// @param Proc is a class that should define the following:
///
/// 1. a typedef for the controlled object
/// typedef ... Proc::ScoredObjType;
///
/// 2. a function to check if the object is ready to send
///    @return >0 object is not ready and want to sleep for retval ms,
///            =0 object is ready.
/// unsigned scoredObjIsReady( unsigned deltaTime, Proc::ScoredObjType& o );
///
/// 3. a function to process the ready object
///    @return >0 object was successfully processed, increment score with retval,
///            <0 object was not processed, increment -retval.
/// int processScoredObj( unsigned deltaTime, Proc::ScoredObjType& o, unsigned& increment );
///
/// 4. dump obj into string
/// void scoredObjToString( std::string& s, Proc::ScoredObjType& o );
///
template < class Proc > class ScoredList
{
private:
    typedef typename Proc::ScoredObjType Obj;

    struct ScoredObj {
        Obj*     obj;
        unsigned score;

        inline ScoredObj( Obj* theObj, unsigned theScore ) :
        obj(theObj), score(theScore) {}

        inline bool operator < ( const ScoredObj& o ) const {
            if ( score < o.score ) return true;
            return false;
        }

        struct isEqual {
            inline isEqual( Obj* o ) : obj(o) {}
            inline bool operator () ( const ScoredObj& o ) { return o.obj == obj; }
            Obj* obj;
        };
    };

    typedef std::vector< ScoredObj >          ObjVector;

public:

    ScoredList( Proc& proc ) : proc_(proc) {}

    /// process one item from objects.
    /// the ready object is searched and then one item from this object is processed.
    /// if the object is processed successfully, the score is incremented,
    /// otherwise score is also incremented for -increment.
    /// if no object is ready or has been processed then
    /// the time to sleep (ms) is returned (>0), otherwise 0 is returned.
    unsigned processOnce( unsigned deltaTime, unsigned sleepTime )
    {
        unsigned wantToSleep = sleepTime;
        movedObjects_.clear();
        bool needsClean = false;
        try {
            for ( typename ObjVector::iterator i = objects_.begin();
                  i != objects_.end();
                  ++i ) {

                if ( ! i->obj ) {
                    needsClean = true;
                    continue;
                }

                unsigned objSleep = proc_.scoredObjIsReady( deltaTime, *i->obj );

                if ( objSleep > 0 ) {
                    // object is not ready
                    if ( objSleep < sleepTime ) sleepTime = objSleep;
                    continue;
                }

                // object is ready
                int increment = proc_.processScoredObj( deltaTime, *i->obj );
                if ( increment < 0 ) {
                    // could not process
                    i->score += unsigned(-increment);
                    movedObjects_.push_back(*i);
                    i->obj = 0;
                    continue;
                }

                // object is processed
                i->score += unsigned(increment);
                movedObjects_.push_back(*i);
                i->obj = 0;
                wantToSleep = 0;
                break;
            }

        } catch (...) {
            postProcess( needsClean );
            throw;
        }
        postProcess(needsClean);
        return wantToSleep;
    }


    void dump( std::string& s ) const {
        char buf[30];
        for ( typename ObjVector::const_iterator i = objects_.begin();
              i != objects_.end();
              ++i ) {
            if ( ! i->obj ) continue;
            sprintf(buf,"\n %3u. sco=%6u ",
                    unsigned(std::distance(objects_.begin(),i)),
                    i->score );
            s.append(buf);
            proc_.scoredObjToString(s,*i->obj);
        }
    }


    void add( Obj* object ) {
        // add an object to the end of the list
        if ( ! object ) return;
        objects_.push_back( ScoredObj(object, objects_.empty() ? 0 : objects_.back().score ) );
    }


    void erase( Obj* object ) {
        typename ObjVector::iterator i =  std::find_if( objects_.begin(),
                                                        objects_.end(),
                                                        typename ScoredObj::isEqual(object) );
        if ( i != objects_.end() ) {
            objects_.erase(i);
        }
    }

    void clear() {
        objects_.clear();
    }

    inline size_t size() const { return objects_.size(); }

    inline Obj* operator [] ( size_t i ) const {
        const ScoredObj& o = objects_.at(i);
        return o.obj;
    }

protected:

    void postProcess( bool needsClean ) {
        if ( ! movedObjects_.empty() ) {
            needsClean = true;
        }
        if ( needsClean ) {
            objects_.erase( std::remove_if(objects_.begin(),
                                           objects_.end(),
                                           typename ScoredObj::isEqual(0)),
                            objects_.end() );
        }
        if ( ! movedObjects_.empty() ) {
            // FIXME: have to restrict on too big difference b/w top and bottom?
            std::sort( movedObjects_.begin(), movedObjects_.end() );
            objects_.reserve( objects_.size() + movedObjects_.size() );
            typename ObjVector::iterator middle = objects_.end();
            std::copy( movedObjects_.begin(), movedObjects_.end(),
                       std::back_inserter(objects_) );
            std::inplace_merge( objects_.begin(), middle, objects_.end() );
            // FIXME: have to check on score overflow
        }
    }


private:
    Proc&     proc_;
    ObjVector objects_;
    ObjVector movedObjects_; // temporary vector is here to hold allocated space
};

}
}
}

#endif
