#ident "$Id$"
#ifndef SMSC_INMAN_IAPLOADER_HPP
#define SMSC_INMAN_IAPLOADER_HPP

#include "util/config/ConfigView.h"
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;
using smsc::logger::Logger;

#include "inman/abprov/IAProvider.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

typedef enum {
    iapCACHE = 0, iapIN, iapHLR, iapDB
} IAProviderType;

typedef enum {
    abContract = 0x01, abSCF = 0x02, abContractSCF = 0x03
} IAProviderAbility_e;

struct IAProvider {
    std::string     ident;
    IAProviderType  type;
    IAProviderITF * prov;   //none for iapIN
};

//typedef std::list<IAProvider> IAProvidersList;


class IAProviderCreatorITF {
public:
    virtual IAProviderType      type(void) const = 0;
    virtual IAProviderAbility_e ability(void) const = 0;
    virtual const char *        ident(void) const = 0;
    virtual IAProviderITF *     create(Logger * use_log) = 0;
    virtual void                logConfig(Logger * use_log) const = 0;
};

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
    <param name="type" type="string">IAProviderType</param>
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

