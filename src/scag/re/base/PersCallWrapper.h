#ifndef _SCAG_RE_BASE_PERSCALLWRAPPER_H
#define _SCAG_RE_BASE_PERSCALLWRAPPER_H

#include "LongCallContextBase.h"

// it is very ugly to include this file here
// because it is in implementation package of other system.
// it should be moved to a separate package someday.
#include "scag/pers/util/PersCallParams.h"

namespace scag2 {
namespace lcm {

class PersCallWrapper : public pers::util::PersCall
{
public:
    PersCallWrapper( pers::util::ProfileType  pt,
                     pers::util::PersCommand* cmd ) :
    ctx_(0), data_(pt,cmd) {}
    virtual pers::util::PersCallData& data() { return data_; }
    virtual void continuePersCall( bool drop ) {
        assert( ctx_ );
        ctx_->initiator->continueExecution( ctx_, drop );
    }
private:
    PersCallWrapper();
    PersCallWrapper( const PersCallWrapper& );
    PersCallWrapper& operator = ( const PersCallWrapper& );

public:
    LongCallContextBase*     ctx_;
private:
    pers::util::PersCallData data_;
};


class PersCallParams : public LongCallParams
{
public:
    PersCallParams( pers::util::ProfileType  pt,
                    pers::util::PersCommand* cmd ) :
    perscall_(pt,cmd) {}

    inline pers::util::PersCall* getPersCall() { return &perscall_; }
    inline void setContext( LongCallContextBase* ctx ) { perscall_.ctx_ = ctx; }
private:
    PersCallWrapper perscall_;
};


}
}

#endif
