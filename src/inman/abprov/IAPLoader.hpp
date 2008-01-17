#pragma ident "$Id$"
/* ************************************************************************** *
 * Abonent Provider shared library loader.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPLOADER_HPP
#define SMSC_INMAN_IAPLOADER_HPP

#include "util/config/ConfigView.h"
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

#include "inman/abprov/IAProvider.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

extern const char * const _IAPTypes[];      //according to IAProviderType
extern const char * const _IAPAbilities[];  //according to IAProviderAbility_e

//This is the Abonent Provider dynamic library entry point
extern "C" IAProviderCreatorITF * 
    loadupAbonentProvider(ConfigView* provCfg, Logger * use_log)
                                            throw(ConfigException);

class IAProviderLoader {
public:
    static IAProviderCreatorITF *
        LoadIAP(ConfigView* provCfg, Logger * use_log) throw(ConfigException);

/* LoadIAP expects Provider configuration formed as follow:

<section name="SampleProvider">
    <param name="loadup" type="string">libinman_iap_db.so</param>
    <section name="Config">
         <!-- SampleProvider configuration -->
    </section>
</section>
*/
protected:
    void* operator new(size_t);
    IAProviderLoader() { }
    ~IAProviderLoader() { }
};

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPLOADER_HPP */

