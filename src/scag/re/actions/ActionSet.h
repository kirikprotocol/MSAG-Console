#ifndef __SCAG_RULE_ENGINE_ACTION_SET__
#define __SCAG_RULE_ENGINE_ACTION_SET__

#include "ActionContext.h"
#include "Action.h"

#include <scag/re/actions/IParserHandler.h>
#include <core/buffers/IntHash.hpp>

#include <iostream>

using smsc::core::buffers::IntHash;
using namespace std;


namespace scag { namespace re { namespace actions {

class ActionSet : public Action
{
    ActionSet(const ActionSet &);
    std::string Variable;
    std::string Value;
protected:
    virtual void StartXMLSubSection(const std::string& name,const SectionParams& params) {};
    virtual void FinishXMLSubSection(const std::string& name) {};
public:
    virtual bool run(ActionContext& context);

    ActionSet();
    virtual void init(const SectionParams& params);
    virtual ~ActionSet();

};



}}}


#endif
