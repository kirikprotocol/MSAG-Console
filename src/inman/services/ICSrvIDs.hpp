/* ************************************************************************** *
 * INMan configurable services ids. 
 * ************************************************************************** */
#ifndef __INMAN_CONFIGURABLE_SERVICES_IDS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_CONFIGURABLE_SERVICES_IDS_HPP__

#include <string>
#include <set>
#include <map>

namespace smsc  {
namespace inman {

//Singleton for converting ISCUid to/from service name
//NOTE: the basic services ids are enumerated first!
class ICSIdent {
public:
    enum UId { icsIdUnknown = 0
        , icsIdSvcHost
        , icsIdTCPServer
        , icsIdTimeWatcher
        , icsIdScheduler
        , icsIdAbntCache
        , icsIdTCAPDisp
        , icsIAPrvdSRI
        , icsIAPrvdATSI
        , icsIdIAPManager
        , icsIdSmBilling
        , icsIdAbntDetector
    };

protected:
    typedef std::map<UId, const char *> ICSNames;
    ICSNames    ids;

    ICSIdent()
    {
        ids.insert(ICSNames::value_type(icsIdUnknown, "icsUnknown"));
        ids.insert(ICSNames::value_type(icsIdSvcHost, "icsSvcHost"));
        ids.insert(ICSNames::value_type(icsIdTCPServer, "icsTCPServer"));
        ids.insert(ICSNames::value_type(icsIdTimeWatcher, "icsTimeWatcher"));
        ids.insert(ICSNames::value_type(icsIdScheduler, "icsScheduler"));
        ids.insert(ICSNames::value_type(icsIdAbntCache, "icsAbntCache"));
        ids.insert(ICSNames::value_type(icsIdTCAPDisp, "icsTCAPDisp"));
        ids.insert(ICSNames::value_type(icsIAPrvdSRI, "icsIAPrvdSRI"));
        ids.insert(ICSNames::value_type(icsIAPrvdATSI, "icsIAPrvdATSI"));
        ids.insert(ICSNames::value_type(icsIdIAPManager, "icsIAPManager"));
        ids.insert(ICSNames::value_type(icsIdSmBilling, "icsSmBilling"));
        ids.insert(ICSNames::value_type(icsIdAbntDetector, "icsAbntDetector"));
    }
    ~ICSIdent()
    { }
    void* operator new(size_t);

    UId _name2UId(const std::string & nm) const
    {
        for (ICSNames::const_iterator it = ids.begin();
                                        it != ids.end(); ++it) {
            if (!nm.compare(it->second))
                return it->first;
        }
        return icsIdUnknown;
    }
    const char * _uid2Name(UId use_uid) const
    {
        ICSNames::const_iterator it = ids.find(use_uid);
        return (it != ids.end()) ? it->second : "icsUnknown";
    }

    static ICSIdent & get(void)
    {
        static ICSIdent instance;
        return instance;
    }

public:
    static UId name2UId(const std::string & nm)
    {
        return ICSIdent::get()._name2UId(nm);
    }
    static const char * uid2Name(UId use_uid)
    {
        return ICSIdent::get()._uid2Name(use_uid);
    }
};

typedef ICSIdent::UId       ICSUId;

//set of ICSUIds sorted starting from basic up to complex ones
class ICSIdsSet : public std::set<ICSUId> {
public:
    bool exist(ICSUId use_uid)
    {
        return (find(use_uid) != end()) ? true : false;
    }
    std::string & toString(std::string & ostr) const
    {
        if (!empty()) {
            ICSIdsSet::const_iterator it = begin();
            ostr += ICSIdent::uid2Name(*it);
            for (++it; it != end(); ++it) {
                ostr += ", "; 
                ostr += ICSIdent::uid2Name(*it);
            }
        }
        return ostr;
    }

    std::string toString(void) const
    {
        std::string ostr;
        toString(ostr);
        return ostr;
    }
};

} //inman
} //smsc
#endif /* __INMAN_CONFIGURABLE_SERVICES_IDS_HPP__ */

