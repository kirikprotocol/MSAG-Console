#ifndef __SCAG_RULE_ENGINE_ACTION_BINOPERATIONS__
#define __SCAG_RULE_ENGINE_ACTION_BINOPERATIONS__

#include "ActionContext.h"
#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionBinOperation : public Action
{
    ActionBinOperation(const ActionBinOperation &);
    std::string strVariable;

    std::string strValue;
    std::string wstrValue;
    bool m_hasValue;

    FieldType valueFieldType;
protected:
    std::string m_ActionName;

    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
    virtual int processOperation(int variable, int value) = 0;
public:
    ActionBinOperation() :m_hasValue(false) {}
    virtual bool run(ActionContext& context);

    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

class ActionInc : public ActionBinOperation
{
protected:
    virtual int processOperation(int variable, int value)
    {
        return variable + value;
    }
public:
    ActionInc() {m_ActionName = "inc";}
};

class ActionDec : public ActionBinOperation
{
protected:
    virtual int processOperation(int variable, int value)
    {
        return variable - value;
    }
public:
    ActionDec() {m_ActionName = "dec";}
};


}}}


#endif
