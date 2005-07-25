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
    IntHash<Action *> actions;
    HandlerType handlerType;
    HandlerType StrToHandlerType(const std::string& str);
protected:
//////////////IParserHandler Interfase///////////////////////
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////
public:
    EventHandler() : handlerType(htUnknown) {};

    virtual ~EventHandler();
    virtual void init(const SectionParams& params);
    inline HandlerType GetHandlerType() const {return handlerType;};

     /** 
     * Processes action (or actions set).
     * Creates ActionContext with transport specific CommandAdapter.
     * Returns RuleStatus from context after action(s) execution.
     * @param   command     command to process
     * @return  status      action(s) execution status
     */
    virtual RuleStatus process(SCAGCommand command);


};
}}
#endif
