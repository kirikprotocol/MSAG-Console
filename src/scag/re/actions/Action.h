#ifndef __SCAG_RULE_ENGINE_ACTION__
#define __SCAG_RULE_ENGINE_ACTION__

#include "ActionContext.h"
#include <scag/re/actions/IParserHandler.h>
#include <core/buffers/IntHash.hpp>

#include <iostream>

using smsc::core::buffers::IntHash;
using namespace std;

namespace scag { namespace re { namespace actions 
{
    /**
     * Abstract action interface to run on ActionContext
     */
class Action : public IParserHandler
{
    //virtual void run(ActionContext& context) = 0;
protected:
};


//////////////////////////////////////////////////////////////////////////


class ActionReturn : public Action
{
    ActionReturn(const ActionReturn &);
public:
    ActionReturn(const SectionParams& params);
    ~ActionReturn();

};

class ActionSet : public Action
{
    ActionSet(const ActionSet &);
    std::string Variable;
    std::string Value;
public:
    ActionSet(const SectionParams& params);
    ~ActionSet();
};

class ActionIf : public Action
{
    ActionIf(const ActionIf &);

    std::string Variable;
    std::string Value;
    std::string Operation;

    bool ActivateThenSection;
    bool ActivateElseSection;
    IntHash<Action *> ThenActions;
    IntHash<Action *> ElseActions;
protected:
//////////////IParserHandler Interfase///////////////////////
    virtual void StartXMLSubSection(const std::string& name,const SectionParams& params);
    virtual void FinishXMLSubSection(const std::string& name);
    virtual bool SetChildObject(const IParserHandler * child);
//////////////IParserHandler Interfase///////////////////////
public:
    ActionIf(const SectionParams& params);
    ~ActionIf();
};


}}}

#endif // __SCAG_RULE_ENGINE_ACTION__

