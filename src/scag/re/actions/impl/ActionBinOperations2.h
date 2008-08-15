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
public:
    ActionBinOperation() :m_hasValue(false), m_valueRequired(false) {}
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
        return (variable + value);
    }
public:
    ActionInc() {m_ActionName = "inc";}
};

class ActionDec : public ActionBinOperation
{
protected:
    virtual int processOperation(int variable, int value)
    {
        return (variable - value);
    }
public:
    ActionDec() {m_ActionName = "dec";}
};


class ActionMul : public ActionBinOperation
{
protected:
    virtual int processOperation(int variable, int value)
    {
        return (variable * value);
    }
public:
    ActionMul() {m_ActionName = "mul"; m_valueRequired = true;}
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
    ActionMod() {m_ActionName = "mod"; m_valueRequired = true;}
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
    ActionDiv() {m_ActionName = "div"; m_valueRequired = true;}
};


}}}


#endif
