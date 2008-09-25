#ifndef _IPARSER_HANDLER2_H_
#define _IPARSER_HANDLER2_H_

#include <core/buffers/Hash.hpp>
#include "scag/exc/SCAGExceptions.h"
#include "scag/transport/TransportType.h"


namespace scag2 {
namespace re {
namespace actions {
    class ActionFactory;
}}}


namespace scag2 {
namespace re { 

using namespace scag::exceptions;
using transport::TransportType;
using re::actions::ActionFactory;

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
