#ifndef SCAG_TRANSPORT_HTTP_ROUTER_TYPES
#define SCAG_TRANSPORT_HTTP_ROUTER_TYPES

#include <string>

#include <xercesc/sax/HandlerBase.hpp>

#include <sms/sms_const.h>
#include <sms/sms.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/XHash.hpp>
#include <util/crc32.h>

#include "HttpCommand.h"

namespace scag { namespace transport { namespace http {

XERCES_CPP_NAMESPACE_USE

using smsc::sms::MAX_ADDRESS_VALUE_LENGTH;
using smsc::sms::Address;
using smsc::core::buffers::Array;
using smsc::core::buffers::XHash;
using smsc::util::crc32;

namespace PlacementType{
    const uint32_t COOKIE = 1;
    const uint32_t HEADER = 2;
    const uint32_t URL = 3;
    const uint32_t PARAM = 4;
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

class AddressMask{
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
};

class AddressURLKey
{
public:
    uint32_t port;
    std::string site;
    std::string path;
    AddressMask mask;

    AddressURLKey() : path(""), mask(""), site(""), port(0)
    {
    }

    AddressURLKey(const std::string& _msk, const std::string& _site, const std::string& _path, uint32_t _port) : site(_site), path(_path), mask(_msk.c_str()), port(_port)
    {
    }

    AddressURLKey(const AddressURLKey& cp) : site(cp.site), path(cp.path), port(cp.port), mask(cp.mask)
    {
    }

    AddressURLKey& operator=(const AddressURLKey& cp)
    {
        site = cp.site;
        path = cp.path;
        port = cp.port;
        mask = cp.mask;
        return *this;
    }

    bool operator==(const AddressURLKey& cp)const
    {
        return mask == cp.mask && site == cp.site && path == cp.path && port == cp.port;
    }

    uint32_t HashCode()const
    {
        return crc32(crc32(crc32(mask.HashCode(), site.c_str(), site.length()), path.c_str(), path.length()), &port, sizeof(uint32_t));
    }

    static uint32_t CalcHash(const AddressURLKey& cp)
    {
        return cp.HashCode();
    }
};

typedef Array<std::string> StringArray;

struct Site{
    std::string host;
    uint32_t port;
    StringArray paths;

    Site(): host(""), port(0) {};

    Site(const Site& cp)
    {
        host = cp.host;
        port = cp.port;
        paths = cp.paths;
    }

    std::string toString()
    {
        std::string s, s1;
        char buf[20];
        sprintf(buf, ":%d", port);
        s1 = host + ((port != 80) ? buf : "");
        for(int i = 0; i < paths.Count(); i++)
            s += s1 + paths[i] + "\n";
        return s;
    }
};

typedef Array<Site> SiteArray;

struct HttpRoute
{
    uint32_t service_id;
    uint32_t provider_id;
    std::string id;
    PlacementArray inUSRPlace;
    PlacementArray outUSRPlace;
    PlacementArray outAddressPlace;

    HttpRoute(): service_id(0), provider_id(0) {}

    HttpRoute(const HttpRoute& cp)
    {
        service_id =cp.service_id;
        provider_id =cp.provider_id;
        id = cp.id;
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

        s += "RouteId: " + id;
        sprintf(buf, " ServiceId: %d\n", service_id);
        s += buf;

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
