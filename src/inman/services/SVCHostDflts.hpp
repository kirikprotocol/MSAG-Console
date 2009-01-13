/* ************************************************************************** *
 * ICServices host loadUp facility definitions and helpers.
 * ************************************************************************** */
#ifndef __INMAN_SVCHOST_DEFAULTS_HPP__
#ident "@(#)$Id$"
#define __INMAN_SVCHOST_DEFAULTS_HPP__

#include <assert.h>

#include <list>

#include "util/config/ConfigException.h"
using smsc::util::config::ConfigException;

#include "inman/common/ObjRegistryT.hpp"
using smsc::util::POBJRegistry_T;

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/ICSrvLoader.hpp"

namespace smsc  {
namespace inman {

//ICService loadUp configuration:
// linkage type ( static - reference to producer,
//                dynamic - name of dynamic library),
// optional config file section name.
struct ICSLoadupCFG {
    enum LDType { //linkage type
        icsLdNone = 0x0
        , icsLdLib = 0x01   //static linkage
        , icsLdDll = 0x02   //dynamic linkage
    };
    ICSUId          icsUId; //
    LDType          ldType;
    std::string     secNm;  //optional XML config section name
    std::string     dllNm;  //shared library name in case of dynamic linkage 
    ICSProducerAC * prod;   //producer in case of static linkage 

    ICSLoadupCFG()
        : icsUId(ICSIdent::icsIdUnknown), ldType(icsLdNone), prod(0)
    { }
    ICSLoadupCFG(ICSUId ics_uid, ICSProducerAC * use_prod,
                 const char * nm_sec = NULL)
        : icsUId(ics_uid), ldType(icsLdLib), prod(use_prod)
    {
        if (nm_sec)
            secNm = nm_sec;
    }
    ICSLoadupCFG(ICSUId ics_uid, const char * nm_dll, const char * nm_sec = NULL)
        : icsUId(ics_uid), ldType(icsLdDll), dllNm(nm_dll), prod(0)
    {
        if (nm_sec)
            secNm = nm_sec;
    }
    ~ICSLoadupCFG()
    { }

    static const char * nmLDType(LDType ld_type)
    {
        if (ld_type == icsLdDll)
            return "dynamic";
        if (ld_type == icsLdLib)
            return "static";
        return "none";
    }
    inline const char * nmLDType(void) const { return nmLDType(ldType); }

    std::string Details(void) const
    {
        std::string res("LoadUp[");
        res += ICSIdent::uid2Name(icsUId); res += "]: ";
        res += nmLDType();
        if (!dllNm.empty()) {
            res += "("; res += dllNm; res += ")";
        }
        res += ", cfg(";
        res += secNm.empty() ? "none" : secNm.c_str();
        res += ")";
        return res;
    }
};

class ICSLoadupList : public std::list<ICSLoadupCFG> {
public:
    ICSLoadupCFG * find(ICSUId ics_id)
    {
        for (ICSLoadupList::iterator it = begin(); it != end(); ++it) {
            if (it->icsUId == ics_id)
                return it.operator->();
        }
        return NULL;
    }
};


//Registry of known ICServices loadups and producers.
class ICSLoadupsReg : public POBJRegistry_T<ICSUId, ICSLoadupCFG> {
protected:
    typedef POBJRegistry_T<ICSUId, ICSProducerAC> ICSProducersReg;

    ICSIdsSet       dfltIds; //services loaded by default
    ICSProducersReg prdcReg; //actual producers registry

    void insLoadUp(ICSLoadupCFG * ld_up)
    {
        insert(ld_up->icsUId, ld_up);
        if (ld_up->prod)
            prdcReg.reset(ld_up->icsUId, ld_up->prod);
    }

    ICSLoadupsReg();
    ~ICSLoadupsReg()
    { }
    void* operator new(size_t);
    
public:
    static ICSLoadupsReg & get(void)
    {
        static ICSLoadupsReg instance;
        return instance;
    }

    ICSProducerAC * getProducer(ICSUId ics_uid) throw(ConfigException)
    {
        ICSProducerAC * prod = prdcReg.find(ics_uid);
        if (!prod) {
            ICSLoadupCFG * ldUp = find(ics_uid);
            if (ldUp) {
                if (!ldUp->prod && (ldUp->ldType == ICSLoadupCFG::icsLdDll)) {
                    prod = ldUp->prod = ICSrvLoader::LoadICS(ldUp->dllNm.c_str()); //throws
                } else
                    prod = ldUp->prod;
                if (prod)
                    prdcReg.insert(ics_uid, prod);
            }
        }
        return prod;
    }

    void resetProducer(ICSUId ics_uid, ICSProducerAC * use_prod)
    {
        prdcReg.reset(ics_uid, use_prod);
        ICSLoadupCFG * ldUp = find(ics_uid);
        if (ldUp)
            ldUp->prod = use_prod;
    }

    //Appends loadUps for services loaded by default, which are absent in ld_list
    //Returns iterator of first added loadUp
    ICSLoadupList::iterator appendDefaults(ICSLoadupList & ld_list) const
    {
        ICSLoadupList::iterator fit = ld_list.end();
        for (ICSIdsSet::const_iterator it = dfltIds.begin();
                                        it != dfltIds.end(); ++it) {
            const ICSLoadupCFG * icsLdUp = find(*it);
            assert(icsLdUp);
            if (!ld_list.find(*it)) {
                ld_list.push_back(*icsLdUp);
                if (fit == ld_list.end())
                    --fit;
            }
        }
        return fit;
    }

    ICSIdsSet knownUIds(void) const
    {
        ICSIdsSet ids;
        for (TRegistry::const_iterator it = registry.begin(); it != registry.end(); ++it)
            ids.insert(it->first);
        return ids;
    }
};


} //inman
} //smsc
#endif /* __INMAN_SVCHOST_DEFAULTS_HPP__ */

