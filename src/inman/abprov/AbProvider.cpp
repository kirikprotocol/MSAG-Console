static const char ident[] = "$Id$";

#include <dlfcn.h>
#include "inman/abprov/AbProvider.hpp"

namespace smsc {
namespace inman {
namespace abprov {

static const char * _fnProvLoader = "loadupAbonentProvider";

typedef AbonentProviderCreatorITF * (*PFAbonentProviderLoader)(ConfigView* provCfg);

/* ************************************************************************** *
 * class DBAbonentProvider implementation:
 * ************************************************************************** */
const AbonentProviderCreatorITF *
    AbonentProviderLoader::LoadDL(const char * dlname, ConfigView* provCfg) throw(ConfigException)
{
    AbonentProviderCreatorITF * cfg = NULL;
    void * dlhandle = dlopen(dlname, RTLD_LAZY);

    if (dlhandle) {
        PFAbonentProviderLoader fnhandle =
            (PFAbonentProviderLoader)dlsym(dlhandle, _fnProvLoader);
        if (fnhandle) {
            try {
                cfg = (*fnhandle)(provCfg); //throws
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

} //abprov
} //inman
} //smsc


