/* ************************************************************************** *
 * Abonent Provider shared library loader.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPLOADER_HPP
#ident "@(#)$Id$"
#define SMSC_INMAN_IAPLOADER_HPP

#include "inman/common/XCFView.hpp"

#include "inman/abprov/IAProvider.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

using smsc::util::config::XConfigView;
using smsc::util::config::ConfigException;

extern const char * const _IAPTypes[];      //according to IAProviderType
extern const char * const _IAPAbilities[];  //according to IAProviderAbility_e

//This is the Abonent Provider dynamic library entry point
extern "C" IAProviderCreatorITF * 
    loadupAbonentProvider(XConfigView* provCfg, Logger * use_log)
                                            throw(ConfigException);

class IAProviderLoader {
public:
    static IAProviderCreatorITF *
        LoadIAP(XConfigView* provCfg, Logger * use_log) throw(ConfigException);

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

