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
#include "SAX2Print.hpp"
#include "scag/transport/SCAGCommand.h"

namespace scag { namespace re
{
using scag::transport::SCAGCommand;
using namespace smsc::core::buffers;
//using smsc::util::config::ConfigView;
//using smsc::scag::admin::SCAGCommand;
using namespace scag::re::actions;
using namespace scag::util::properties;

typedef smsc::core::buffers::Hash<std::string> SectionParams;

class EventHandler : public IParserHandler
{
    EventHandler(const EventHandler &);
    IntHash<Action *> actions;
protected:
//////////////IParserHandler Interfase///////////////////////
    virtual void SetChildObject(IParserHandler * child);
    virtual void StartXMLSubSection(const std::string& name,const SectionParams& params) {};
    virtual void FinishXMLSubSection(const std::string& name) {};
//////////////IParserHandler Interfase///////////////////////
public:
    EventHandler(const SectionParams& params);
    virtual ~EventHandler();

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
    virtual RuleStatus process(SCAGCommand command);


};
}}
#endif
