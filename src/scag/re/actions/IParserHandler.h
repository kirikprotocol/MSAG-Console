#ifndef _IPARSER_HANDLER_H_
#define _IPARSER_HANDLER_H_

#include <core/buffers/Hash.hpp>
#include "scag/exc/SCAGExceptions.h"
#include "scag/transport/SCAGCommand.h"


namespace scag { namespace re { namespace actions {
    class ActionFactory;
}}}


namespace scag { namespace re { 

using namespace scag::exceptions;
using scag::transport::TransportType;
using scag::re::actions::ActionFactory;

typedef smsc::core::buffers::Hash<std::string> SectionParams;


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
    virtual void SetChildObject(IParserHandler * child) {throw SCAGException("IParserHandler object must have no children");}

    virtual void init(const SectionParams& params,PropertyObject propertyObject) = 0;
    virtual ~IParserHandler() {};
};


}}

#endif
