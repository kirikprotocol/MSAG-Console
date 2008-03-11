#ifndef MOD_IDENT_OFF
static const char ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/abprov/IAPLoader.hpp"
#include "inman/common/DLGuard.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
//according to IAProviderType
const char * const _IAPTypes[] = {"CACHE", "IN", "HLR", "DB"};
//according to IAProviderAbility_e
const char * const _IAPAbilities[] = { "none", "abContract", "abSCF", "abContractSCF" };

static const char * _fnProvLoader = "loadupAbonentProvider";

typedef IAProviderCreatorITF * (*PFAbonentProviderLoader)(XConfigView* provCfg, Logger * use_log);

/* ************************************************************************** *
 * class AbonentProviderLoader implementation:
 * ************************************************************************** */
IAProviderCreatorITF *
    IAProviderLoader::LoadIAP(XConfigView* provCfg, Logger * use_log) throw(ConfigException)
{
    IAProviderCreatorITF * cfg = NULL;
    const char * dlname = NULL;
    try { dlname = provCfg->getString("loadup");
    } catch (const ConfigException & exc) { }
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
            std::auto_ptr<XConfigView> dbCfg(provCfg->getSubConfig("Config"));
            cfg = (*fnhandle)(dbCfg.get(), use_log); //throws from .so !!!
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


