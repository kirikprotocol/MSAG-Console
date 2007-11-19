/* $Id$ */

#ifndef __SCAG_BILL_INFRA_XMLHANDLERS_H__
#define __SCAG_BILL_INFRA_XMLHANDLERS_H__

#include <xercesc/sax/HandlerBase.hpp>

#include <util/Exception.hpp>

#include <core/buffers/IntHash.hpp>
#include <logger/Logger.h>

#include "PersSocketServer.h"

namespace scag { namespace cpers {

using namespace smsc::util;
using namespace smsc::logger;
using smsc::core::buffers::Hash;
using scag::pers::ConnectionContext;

XERCES_CPP_NAMESPACE_USE

class RegionInfo
{
public:
  RegionInfo():id(0), ctx(0) {}
  RegionInfo(const RegionInfo& ri):id(ri.id), ctx(ri.ctx), name(ri.name), passwd(ri.passwd) { }
  RegionInfo& operator=(const RegionInfo& ri) {
    if (this == &ri) {
      return *this;
    }
    name = ri.name;
    passwd = ri.passwd;
    id = ri.id;
    ctx = ri.ctx;
    return *this;
  }
public:
    std::string name, passwd;
    uint32_t id;
    ConnectionContext *ctx;
};

class XMLBasicHandler : public HandlerBase
{
    int type;
    Logger * logger;
    IntHash<RegionInfo> *regionHash;

    void characters(const XMLCh *const chars, const unsigned int length);

protected:

public:
    XMLBasicHandler(IntHash<RegionInfo>* h);
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

}}

#endif
