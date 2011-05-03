#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/ICSrvDefs.hpp"
using smsc::core::synchronization::MutexGuard;

namespace smsc  {
namespace inman {
/* ************************************************************************** *
 * class ICServiceAC implementation. 
 * ************************************************************************** */
ICServiceAC::RCode ICServiceAC::checkDeps(State tgt_state)
{
    for (ICSIdsSet::const_iterator it = _icsDeps.begin();
                                it != _icsDeps.end(); ++it) {
        if (_icsHost->getICSrvState(*it) < tgt_state)
            return icsRcInsuff;
    }
    return icsRcOk;
}

//Initializes service verifying that all dependent services are inited
ICServiceAC::RCode ICServiceAC::ICSInit(void)
{
    MutexGuard  grd(_icsSync());
    if (_icsState < ICServiceAC::icsStConfig)
        return ICServiceAC::icsRcError;

    ICServiceAC::RCode rc = ICServiceAC::icsRcOk;
    if (_icsState < ICServiceAC::icsStInited) {
        //check dependent services first
        if ((rc = checkDeps(ICServiceAC::icsStInited)) == ICServiceAC::icsRcOk) {
            //initialize AbonentDetector service
            if ((rc = _icsInit()) == ICServiceAC::icsRcOk)
                _icsState = ICServiceAC::icsStInited;
        }
    }
    return rc;
}
//Starts service verifying that all dependent services are started
ICServiceAC::RCode ICServiceAC::ICSStart(void)
{
    MutexGuard  grd(_icsSync());
    if (_icsState < ICServiceAC::icsStInited)
        return ICServiceAC::icsRcError;

    ICServiceAC::RCode rc = ICServiceAC::icsRcOk;
    if (_icsState < ICServiceAC::icsStStarted) {
        //check dependent services first
        if ((rc = checkDeps(ICServiceAC::icsStStarted)) == ICServiceAC::icsRcOk) {
            //start AbonentDetector service
            if ((rc = _icsStart()) == ICServiceAC::icsRcOk)
                _icsState = ICServiceAC::icsStStarted;
        }
    }
    return rc;
}
//Stops service
void  ICServiceAC::ICSStop(bool do_wait/* = false*/)
{
    MutexGuard  grd(_icsSync());
    if (_icsState > ICServiceAC::icsStInited) {
        _icsStop(do_wait);
        _icsState = ICServiceAC::icsStInited;
    } else if (do_wait && (_icsState == ICServiceAC::icsStInited))
        _icsStop(do_wait);
}


} //inman
} //smsc


