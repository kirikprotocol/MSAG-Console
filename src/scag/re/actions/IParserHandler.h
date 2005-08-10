#ifndef _IPARSER_HANDLER_H_
#define _IPARSER_HANDLER_H_

#include <core/buffers/Hash.hpp>
#include <util/Exception.hpp>
#include "scag/transport/SCAGCommand.h"

namespace scag { namespace re { 

using smsc::util::Exception;
using scag::transport::TransportType;


typedef smsc::core::buffers::Hash<std::string> SectionParams;

class ActionFactory;

struct PropertyObject
{
    TransportType transport;
    int HandlerId;
};

class IParserHandler
{
public:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory) = 0;
    virtual bool FinishXMLSubSection(const std::string& name) = 0;
    virtual void SetChildObject(IParserHandler * child) {throw Exception("IParserHandler object must have no children");};

    virtual void init(const SectionParams& params,PropertyObject propertyObject) = 0;
    virtual ~IParserHandler() {};
};


}}

#endif
