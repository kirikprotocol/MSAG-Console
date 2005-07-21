#ifndef _ACTION_CHOOSE_H_
#define _ACTION_CHOOSE_H_

#include "Action.h"

namespace scag { namespace re { namespace actions {


class ActionChoose : public Action
{
    ActionChoose(const ActionChoose &);

    bool ActivateWhenSection;
    bool ActivateOtherwiseSection;

    IntHash<Action *> WhenActions;
    IntHash<Action *> OtherwiseActions;
protected:
//////////////IParserHandler Interfase///////////////////////
    virtual void StartXMLSubSection(const std::string& name,const SectionParams& params);
    virtual void FinishXMLSubSection(const std::string& name);
    virtual void SetChildObject(IParserHandler * child);
//////////////IParserHandler Interfase///////////////////////
public:
    ActionChoose(const SectionParams& params);
    ~ActionChoose();
};
}}}

#endif
