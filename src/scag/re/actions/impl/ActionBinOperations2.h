#ifndef __SCAG_RULE_ENGINE_ACTION_BINOPERATIONS2__
#define __SCAG_RULE_ENGINE_ACTION_BINOPERATIONS2__

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionBinOperation : public Action
{
    ActionBinOperation(const ActionBinOperation &);
    std::string strVariable;

    std::string strValue;
    bool m_hasValue;

    FieldType valueFieldType;
protected:
    std::string m_ActionName;
    bool m_valueRequired;

    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
    virtual int processOperation(int variable, int value) = 0;

    ActionBinOperation( const char* opname, bool valueRequired = false ) :
    m_ActionName(opname), m_hasValue(false), m_valueRequired(valueRequired) {}

public:
    virtual bool run(ActionContext& context);

    virtual const char* opname() const { return m_ActionName.c_str(); }
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

class ActionInc : public ActionBinOperation
{
protected:
    virtual int processOperation(int variable, int value)
    {
        return (variable + value);
    }
public:
    ActionInc() : ActionBinOperation("inc") {}
};

class ActionDec : public ActionBinOperation
{
protected:
    virtual int processOperation(int variable, int value)
    {
        return (variable - value);
    }
public:
    ActionDec() : ActionBinOperation("dec") {}
};


class ActionMul : public ActionBinOperation
{
protected:
    virtual int processOperation(int variable, int value)
    {
        return (variable * value);
    }
public:
    ActionMul() : ActionBinOperation("mul",true) {}
};



class ActionMod : public ActionBinOperation
{
protected:
    virtual int processOperation(int variable, int value)
    {
        if (value == 0) 
        {
            smsc_log_warn(logger,"Action '%s' cannot process operation. Details: Devision by zero", m_ActionName.c_str());
            return variable;
        }

        return (variable % value);
    }
public:
    ActionMod() : ActionBinOperation("mod",true) {}
};



class ActionDiv : public ActionBinOperation
{
protected:
    virtual int processOperation(int variable, int value)
    {
        if (value == 0) 
        {
            smsc_log_warn(logger,"Action '%s' cannot process operation. Details: Devision by zero", m_ActionName.c_str());
            return variable;
        }
        return (variable / value);
    }
public:
    ActionDiv() : ActionBinOperation("div",true) {}
};


}}}


#endif
