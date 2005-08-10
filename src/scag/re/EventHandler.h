#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#include <list>

#include "XMLHandlers.h"
#include <scag/re/actions/IParserHandler.h>
#include <scag/re/actions/Action.h>
#include "scag/transport/SCAGCommand.h"

namespace scag { namespace re
{
using scag::transport::SCAGCommand;
using namespace scag::re::actions;

class EventHandler : public IParserHandler
{
    EventHandler(const EventHandler &);
    PropertyObject propertyObject;
protected:
    std::list<Action *> actions;
//////////////IParserHandler Interfase///////////////////////
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////
public:
    EventHandler()  {};
    virtual ~EventHandler();

    virtual void init(const SectionParams& params,PropertyObject _propertyObject) {propertyObject = _propertyObject;}
    virtual RuleStatus process(SCAGCommand& command) = 0;
    virtual StrToHandlerId(const std::string& str) = 0;

};

}}
#endif
