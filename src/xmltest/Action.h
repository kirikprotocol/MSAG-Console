#ifndef _ACTION_H_
#define _ACTION_H_

#include "XMLHandlers.h"
#include "util/Exception.hpp"
#include <core/buffers/IntHash.hpp>
#include <iostream>
#include "IParserHandler.h"

using namespace smsc::core::buffers;

class Action : public IParserHandler
{
};

class ActionSet : public Action
{
    ActionSet(const ActionSet &);

public:
    ActionSet(const SectionParams& params);
    ~ActionSet();
};

class ActionIf : public Action
{
    ActionIf(const ActionIf &);

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



#endif
