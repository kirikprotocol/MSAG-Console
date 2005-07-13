#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#include <iostream>
#include <core/buffers/IntHash.hpp>

#include "XMLHandlers.h"
#include <util/Exception.hpp>
#include <scag/re/actions/IParserHandler.h>
#include <scag/re/actions/Action.h>
//#include <util/config/ConfigView.h>
//#include <scag/admin/SCAGCommand.h>

namespace scag { namespace re
{

using namespace smsc::core::buffers;
//using smsc::util::config::ConfigView;
//using smsc::scag::admin::SCAGCommand;
using namespace scag::re::actions;

typedef smsc::core::buffers::Hash<std::string> SectionParams;

class EventHandler : public IParserHandler
{
    EventHandler(const EventHandler &);
    IntHash<Action *> actions;
protected:
//////////////IParserHandler Interfase///////////////////////
    virtual bool SetChildObject(const IParserHandler * child);
//////////////IParserHandler Interfase///////////////////////
public:
    EventHandler(const SectionParams& params);
    ~EventHandler();

    /**
     * Creates & configure action(s) from sub-section (via ActionsFactory)
     * @param   config      config sub-section for handler
     */
//    virtual void init(ConfigView* config) = 0;

    /** 
     * Processes action (or actions set).
     * Creates ActionContext with transport specific CommandAdapter.
     * Returns RuleStatus from context after action(s) execution.
     * @param   command     command to process
     * @return  status      action(s) execution status
     */
//    virtual RuleStatus process(SCAGCommand command) = 0;


};
}}
#endif
