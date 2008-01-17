#ifndef MOD_IDENT_OFF
static const char ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <dlfcn.h>
#include "inman/abprov/IAPLoader.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
//according to IAProviderType
const char * const _IAPTypes[] = {"CACHE", "IN", "HLR", "DB"};
//according to IAProviderAbility_e
const char * const _IAPAbilities[] = { "none", "abContract", "abSCF", "abContractSCF" };

static const char * _fnProvLoader = "loadupAbonentProvider";

typedef IAProviderCreatorITF * (*PFAbonentProviderLoader)(ConfigView* provCfg, Logger * use_log);

//NOTE: The DL entry function throws exceptions, so dlclose() should be called
//only after exception cleanUp is finished.
//This guard ensures that DL will be unloaded while stack unbind.
class DLGuard {
public:
    DLGuard(const char * dl_name)   { dlHdl = dlopen(dl_name, RTLD_LAZY); }
    ~DLGuard()                      { if (dlHdl) dlclose(dlHdl); }

    void * getHdl(void) const       { return dlHdl; }
    void release(void)              { dlHdl = NULL; }

protected:
    void * dlHdl;
};

/* ************************************************************************** *
 * class AbonentProviderLoader implementation:
 * ************************************************************************** */
IAProviderCreatorITF *
    IAProviderLoader::LoadIAP(ConfigView* provCfg, Logger * use_log) throw(ConfigException)
{
    IAProviderCreatorITF * cfg = NULL;
    char * dlname = NULL;
    try { dlname = provCfg->getString("loadup");
    } catch (ConfigException& exc) { }
    if (!dlname)
        throw ConfigException("'AbonentProvider' library name is missed!");

    if (!provCfg->findSubSection("Config"))
        throw ConfigException("'Config' subsection is missed!");

    smsc_log_info(use_log, "Loading AbonentProvider driver '%s' ..", dlname);
    DLGuard dlGrd(dlname);
    if (dlGrd.getHdl()) {
        PFAbonentProviderLoader fnhandle =
            (PFAbonentProviderLoader)dlsym(dlGrd.getHdl(), _fnProvLoader);
        if (fnhandle) {
            ConfigView* dbCfg = provCfg->getSubConfig("Config");
            cfg = (*fnhandle)(dbCfg, use_log); //throws from .so !!!
        } else {
            throw ConfigException("dlsym() failed for: %s", _fnProvLoader);
        }
    } else
        throw ConfigException("dlopen() failed to load %s, reason %s ", dlname, dlerror());
    dlGrd.release();
    return cfg;
}

} //iaprvd
} //inman
} //smsc


