/* $Id$ */

#ifndef __SCAG_TRANSPORT_HTTP_XMLHANDLERS_H__
#define __SCAG_TRANSPORT_HTTP_XMLHANDLERS_H__

#include <xercesc/sax/HandlerBase.hpp>

#include <util/Exception.hpp>

#include <core/buffers/IntHash.hpp>
#include <logger/Logger.h>

#include "RouterTypes.h"

namespace scag { namespace transport { namespace http {

using namespace smsc::util;
using namespace smsc::logger;
using smsc::core::buffers::Hash;

XERCES_CPP_NAMESPACE_USE

class XMLBasicHandler : public HandlerBase
{
    Hash<StringArray> subj_hash;
    bool in_route, in_subj_def;
    std::string route_id, url;
    uint32_t service_id;
    Array<std::string> addrs;

    std::string subj_id;
    RouteArray* routes;
    HttpRoute route;

    void characters(const XMLCh *const chars, const unsigned int length);

    Logger *logger;

public:
    XMLBasicHandler(RouteArray*);
    ~XMLBasicHandler() {};

    void startElement(const XMLCh* const qname, AttributeList& attributes);
    void endElement(const XMLCh* const qname);

    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
};

class StrX
{
public :
    StrX(const XMLCh* const toTranscode)
    {
        fLocalForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
        XMLString::release(&fLocalForm);
    }

    const char* localForm() const
    {
        return fLocalForm;
    }

private :
    char* fLocalForm;
};

}}}

#endif
