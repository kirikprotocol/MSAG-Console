static const char ident[] = "$Id$";

#include <dlfcn.h>
#include "inman/abprov/IAPLoader.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {

static const char * _fnProvLoader = "loadupAbonentProvider";

typedef IAProviderCreatorITF * (*PFAbonentProviderLoader)(ConfigView* provCfg, Logger * use_log);

/* ************************************************************************** *
 * class AbonentProviderLoader implementation:
 * ************************************************************************** */
const IAProviderCreatorITF *
    IAProviderLoader::LoadIAP(ConfigView* provCfg, Logger * use_log) throw(ConfigException)
{
    IAProviderCreatorITF * cfg = NULL;
    char * dlname = NULL;
    try {
        dlname = provCfg->getString("loadup");
    } catch (ConfigException& exc) {
        throw ConfigException("'AbonentProvider' library name is missed!");
    }

    smsc_log_info(use_log, "Loading AbonentProvider driver '%s' ..", dlname);
    void * dlhandle = dlopen(dlname, RTLD_LAZY);
    if (dlhandle) {
        PFAbonentProviderLoader fnhandle =
            (PFAbonentProviderLoader)dlsym(dlhandle, _fnProvLoader);
        if (fnhandle) {
            try {
                if (!provCfg->findSubSection("Config"))
                    throw ConfigException("'Config' subsection is missed!");
                ConfigView* dbCfg = provCfg->getSubConfig("Config");
                cfg = (*fnhandle)(dbCfg, use_log); //throws
            } catch (ConfigException & exc) {
                dlclose(dlhandle); 
                throw exc;
            }
        } else {
            dlclose(dlhandle);
            throw ConfigException("dlsym() failed for: %s", _fnProvLoader);
        }
    } else
        throw ConfigException("dlopen() failed to load %s, reason %s ", dlname, dlerror());
    return cfg;
}

} //iaprvd
} //inman
} //smsc


