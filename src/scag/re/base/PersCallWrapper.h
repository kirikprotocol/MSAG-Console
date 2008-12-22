#ifndef _SCAG_RE_BASE_PERSCALLWRAPPER_H
#define _SCAG_RE_BASE_PERSCALLWRAPPER_H

#include "LongCallContextBase.h"

// it is very ugly to include this file here
// because it is in implementation package of other system.
// it should be moved to a separate package someday.
#include "scag/pvss/base/PersCall.h"

namespace scag2 {
namespace lcm {

class PersCallParams : public LongCallParams
{
public:
    PersCallParams( pvss::ProfileType  pt,
                    pvss::PersCommand* cmd ) :
    perscall_(pt,cmd,0) {}
    inline pvss::PersCall* getPersCall() { return &perscall_; }
private:
    pvss::PersCall perscall_;
};


}
}

#endif
