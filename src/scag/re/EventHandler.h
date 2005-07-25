#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#include <core/buffers/IntHash.hpp>

#include "XMLHandlers.h"
#include <scag/re/actions/IParserHandler.h>
#include <scag/re/actions/Action.h>
#include "scag/transport/SCAGCommand.h"

namespace scag { namespace re
{
using scag::transport::SCAGCommand;
using namespace smsc::core::buffers;
using namespace scag::re::actions;

enum HandlerType
{
    htUnknown,
    htDeliver,
    htSubmit
};

class EventHandler : public IParserHandler
{
    EventHandler(const EventHandler &);
protected:
    IntHash<Action *> actions;
    HandlerType handlerType;

//////////////IParserHandler Interfase///////////////////////
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////
public:
    EventHandler() : handlerType(htUnknown) {};
    virtual ~EventHandler();

    virtual void init(const SectionParams& params) = 0;
    virtual RuleStatus process(SCAGCommand command) = 0;

    inline HandlerType GetHandlerType() const {return handlerType;};
    HandlerType StrToHandlerType(const std::string& str);
};

}}
#endif
