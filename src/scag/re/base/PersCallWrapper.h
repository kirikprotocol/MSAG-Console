#ifndef _SCAG_RE_BASE_PERSCALLWRAPPER_H
#define _SCAG_RE_BASE_PERSCALLWRAPPER_H

#include "LongCallContextBase.h"

// it is very ugly to include this file here
// because it is in implementation package of other system.
// it should be moved to a separate package someday.
#include "scag/pers/util/PersCallParams.h"

namespace scag2 {
namespace lcm {

class PersCallParams : public LongCallParams
{
public:
    PersCallParams( pers::util::ProfileType  pt,
                    pers::util::PersCommand* cmd ) :
    perscall_(pt,cmd,0) {}
    inline pers::util::PersCall* getPersCall() { return &perscall_; }
private:
    pers::util::PersCall perscall_;
};


}
}

#endif
