#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/DLGuard.hpp"
using smsc::util::DLGuard;

#include "inman/services/ICSrvLoader.hpp"

namespace smsc {
namespace inman {

static const char * _fnICSLoader = "loadupICService";

typedef ICSProducerAC * (*fpICSLoader)(void);

/* ************************************************************************** *
 * class ICSrvLoader implementation:
 * ************************************************************************** */
ICSProducerAC *
    ICSrvLoader::LoadICS(const char * dl_name) throw(ConfigException)
{
    DLGuard dlGrd(dl_name);
    if (!dlGrd.getHdl())
        throw ConfigException("dlopen() failed to load %s, reason %s ", dl_name, dlerror());

    ICSProducerAC * cfg = NULL;
    fpICSLoader fnhandle = (fpICSLoader)dlsym(dlGrd.getHdl(), _fnICSLoader);
    if (!fnhandle)
        throw ConfigException("dlsym() failed for: %s", _fnICSLoader);
    cfg = (*fnhandle)(); //throws from .so !!!

    dlGrd.release();
    return cfg;
}

} //inman
} //smsc

