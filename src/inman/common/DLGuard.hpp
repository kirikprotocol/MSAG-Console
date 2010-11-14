/* ************************************************************************** *
 * Dynamic Library handle guard (helps in case the DL entry function throws
 * exceptions).
 * ************************************************************************** */
#ifndef __UTIL_DL_GUARD_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UTIL_DL_GUARD_HPP__

#include <dlfcn.h>

namespace smsc {
namespace util {
//NOTE: in case the DL entry function throws exceptions, the dlclose() should
//be called only after exception cleanUp is finished.
//This guard ensures that DL will be unloaded while stack unbind phase.
class DLGuard {
public:
    DLGuard(const char * dl_name, int dl_mode = RTLD_LAZY)
    {
        dlHdl = dlopen(dl_name, dl_mode);
    }
    ~DLGuard()
    {
        if (dlHdl) dlclose(dlHdl); 
    }

    void * getHdl(void) const { return dlHdl; }
    void   release(void)      { dlHdl = 0; }

protected:
    void * dlHdl;
};

} //inman
} //smsc
#endif /* __UTIL_DL_GUARD_HPP__ */

