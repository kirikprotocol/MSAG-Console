#ifndef SCAG_TRANSPORT_HTTP_IMPL_ROUTER_TYPES
#define SCAG_TRANSPORT_HTTP_IMPL_ROUTER_TYPES

#include <string>

#include <xercesc/sax/HandlerBase.hpp>

#include <sms/sms_const.h>
#include <sms/sms.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/XHash.hpp>
#include <util/crc32.h>

#include "scag/transport/http/base/HttpCommand2.h"

namespace scag2 {
namespace transport {
namespace http {

XERCES_CPP_NAMESPACE_USE

using smsc::sms::MAX_ADDRESS_VALUE_LENGTH;
using smsc::sms::Address;
using smsc::core::buffers::Array;
using smsc::core::buffers::XHash;
using smsc::util::crc32;

#define PLACEMENT_TYPE_COUNT 4
namespace PlacementType{
    const uint32_t UNKNOWN = 255;
    const uint32_t COOKIE = 0;
    const uint32_t HEADER = 1;
    const uint32_t URL = 2;
    const uint32_t PARAM = 3;
};
#define PLACEMENT_KIND_COUNT 4
namespace PlacementKind{
    const uint32_t UNKNOWN = 255;
    // const uint32_t USR = 0;
    const uint32_t ADDR = 1;
    const uint32_t ROUTE_ID = 2;
    const uint32_t SERVICE_ID = 3;
};

class Placement{
public:
    uint32_t type;
    uint32_t prio;
    std::string name;
    bool setType(const char* t)
    {
        if(!strcmp(t, "cookie"))
            type = PlacementType::COOKIE;
        else if(!strcmp(t, "header"))
            type = PlacementType::HEADER;
        else if(!strcmp(t, "url"))
            type = PlacementType::URL;
        else if(!strcmp(t, "param"))
            type = PlacementType::PARAM;
        else
            return false;

        return true;
    }
};

typedef Array<Placement> PlacementArray;
typedef PlacementArray PlacementKindArray[PLACEMENT_KIND_COUNT];


/*class AddressMask{
public:
    uint8_t len;
    uint8_t full_len;
    char mask[MAX_ADDRESS_VALUE_LENGTH + 1];

    AddressMask(const char* msk)
    {
        if(*msk == '+')
            msk++;

        full_len = strlen(msk);
        if(full_len >= MAX_ADDRESS_VALUE_LENGTH)
            full_len = MAX_ADDRESS_VALUE_LENGTH;

        len = 0;
        if(full_len > 0)
        {
            const char *ptr = msk + full_len;
            while(ptr > msk && ptr[-1] == '?')
                ptr--;
            len = ptr - msk;
            if(len > 0)
                memcpy(mask, msk, len);
        }
    }

    AddressMask(const AddressMask& am)
    {
        len = am.len;
        full_len = am.full_len;
        memcpy(mask, am.mask, len);
    }

    AddressMask& operator=(const AddressMask& am)
    {
        len = am.len;
        full_len = am.full_len;
        memcpy(mask, am.mask, len);
        return *this;
    }

    uint8_t cut()
    {
        if(!len)
            return 0;

        return --len;
    }

    std::string toString()
    {
        mask[len] = 0;
        std::string s = mask;
        uint8_t i = full_len - len;

        while(i--)
            s += '?';

        return s;
    }

    bool operator==(const AddressMask& am)const
    {
        return full_len == am.full_len && len == am.len && !memcmp(mask, am.mask, len);
    }

    uint32_t HashCode()const
    {
        return crc32(crc32(crc32(1, mask, len), &len, 1), &full_len, 1);
    }

    static uint32_t CalcHash(AddressMask& am)
    {
        return am.HashCode();
    }
};*/

class AddressURLKey
{
public:
    uint32_t maskId, hostId, pathId;

    AddressURLKey() :maskId(0), hostId(0), pathId(0) {}
    AddressURLKey(uint32_t _mid, const uint32_t _hid, const uint32_t _pid) : maskId(_mid), hostId(_hid), pathId(_pid) {}
    AddressURLKey(const AddressURLKey& cp) : maskId(cp.maskId), hostId(cp.hostId), pathId(cp.pathId) {}

    AddressURLKey& operator=(const AddressURLKey& cp)
    {
        maskId = cp.maskId;
        pathId = cp.pathId;
        hostId = cp.hostId;
        return *this;
    }

    bool operator==(const AddressURLKey& cp)const
    {
        return maskId == cp.maskId && hostId == cp.hostId && pathId == cp.pathId;
    }

    uint32_t HashCode()const
    {
        return crc32(crc32(crc32(1, &maskId, sizeof(uint32_t)), &pathId, sizeof(uint32_t)), &hostId, sizeof(uint32_t));
    }

    static uint32_t CalcHash(const AddressURLKey& cp)
    {
        return cp.HashCode();
    }
};

typedef Array<std::string> StringArray;

struct Site{
    bool def;
    std::string host;
    uint32_t port;
    StringArray paths;

    Site(): def(false), host(""), port(0) {};

    Site(const Site& cp)
    {
        host = cp.host;
        port = cp.port;
        paths = cp.paths;
        def = cp.def;
    }

    std::string toString()
    {
        std::string s, s1;
        char buf[20];
        sprintf(buf, ":%d", port);
        s = s1 = host + ((port != 80) ? buf : "") + (def ? " default\n" : "\n");
        for(int i = 0; i < paths.Count(); i++)
            s += paths[i] + "\n";
        return s;
    }
};

typedef Array<Site> SiteArray;

struct HttpRoute
{
    bool enabled, def, transit;
    uint32_t service_id;
    uint32_t provider_id;
    uint32_t id;
    std::string name;
    PlacementKindArray inPlace;
    PlacementKindArray outPlace;
    std::string addressPrefix;
    Site defSite;
    bool statistics;

    HttpRoute():
    enabled(true),
    def(false),
    transit(false),
    service_id(0),
    provider_id(0),
    id(0),
    statistics(true)
    {}

    HttpRoute(const HttpRoute& cp)
    {
        service_id =cp.service_id;
        provider_id =cp.provider_id;
        id = cp.id;
        name = cp.name;
        for(int i = 0; i < PLACEMENT_KIND_COUNT; i++)
        {
            inPlace[i] = cp.inPlace[i];
            outPlace[i] = cp.outPlace[i];            
        }
        enabled = cp.enabled;
        def = cp.def;        
        transit = cp.transit;
        addressPrefix = cp.addressPrefix;
        defSite = cp.defSite;
        statistics = cp.statistics;
    }
};

struct HttpRouteInt : public HttpRoute
{
    StringArray masks;
    SiteArray sites;

    HttpRouteInt(): HttpRoute() {};

    HttpRouteInt(const HttpRouteInt& cp): HttpRoute(cp)
    {
        masks = cp.masks;
        sites = cp.sites;
    }

    std::string toString()
    {
        char buf[100];
        std::string s;

        s += "RouteName: \"" + name + "\"";
        sprintf(buf, " RouteId: %d; ServiceId: %d", id, service_id);
        s += buf;
        
        if(enabled) s += " enabled";
        if(def) s += " default";
        if(transit) s += " transit";
        s += " AddressPrefix: " + addressPrefix + "\n";

        s += "Masks:\n";
        for(int i = 0; i < masks.Count(); i++)
            s += masks[i] + "\n";

        s += "Urls:\n";
        for(int i = 0; i < sites.Count(); i++)
            s += sites[i].toString();

        return s;
    }
};

typedef Array<HttpRouteInt> RouteArray;

}}}

#endif //SCAG_TRANSPORT_HTTP_ROUTER_TYPES
