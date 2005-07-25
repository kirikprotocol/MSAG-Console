#include "ActionIf.h"
#include "scag/re/SAX2Print.hpp"

#include "scag/re/Rule.h"
#include "scag/re/ActionFactory.h"

namespace scag { namespace re { namespace actions {


ActionIf::ActionIf() : FillThenSection(true), FillElseSection(false)
{
}

void ActionIf::init(const SectionParams& params)
{
    if (!params.Exists("test")) throw Exception("Action 'if': missing 'test' parameter");

    singleparam.Operand1 = params["test"];

    const char * name = 0;

    FieldType ft;
    ft = ActionContext::Separate(singleparam.Operand1,name); 
    if (ft==ftUnknown) throw Exception("Action 'if': unrecognized variable prefix ");

    bool hasOP = params.Exists("op");
    bool hasValue = params.Exists("value");

    if (hasOP&&hasValue) 
    {
        singleparam.Operation = GetOperationFromSTR(params["op"]);
        singleparam.Operand2 = params["value"];
        if (singleparam.Operation == opUnknown) throw Exception("Action 'if': unrecognized operation");
        if (singleparam.Operand2.size()==0) throw Exception("Action 'if': invalid 'value' parameter");
    } else 
    {
        if (hasOP&&(!hasValue)) throw Exception("Action 'if': missing 'value' parameter"); 
        if ((!hasOP)&&hasValue) throw Exception("Action 'if': missing 'op' parameter"); 
    }
}

ActionIf::~ActionIf()
{
    int key;
    Action * value;

    for (IntHash<Action *>::Iterator it = ThenActions.First(); it.Next(key, value);)
    {
        delete value;
    }

    for (IntHash<Action *>::Iterator it = ElseActions.First(); it.Next(key, value);)
    {
        delete value;
    }
    smsc_log_debug(logger, "'if' action released");
}


IParserHandler * ActionIf::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    if (name == "then")
    {
        FillThenSection = true;
        FillElseSection = false;
        return 0;
    } else if (name == "else") 
    {
        FillThenSection = false;
        FillElseSection = true;
        return 0;
    } 
    else
    {
        Action * action = 0;
        action = factory.CreateAction(name);
        if (!action) 
        {
            std::string msg("EventHandler: unrecognized child object '");
            msg.append(name);
            msg.append("' to create");
            throw Exception(msg.c_str());
        }

        try
        {
            action->init(params);
        } catch (Exception& e)
        {
            delete action;
            throw e;
        }

        if (FillThenSection) 
        {
            ThenActions.Insert(ThenActions.Count(),action);
            smsc_log_debug(logger,"Action 'if': child object set to 'then'");
        }
        else if (FillElseSection) 
        {
            ElseActions.Insert(ElseActions.Count(),action);
            smsc_log_debug(logger,"Action 'if': child object set to 'else'");
        }

        return action;
    }
}

bool ActionIf::FinishXMLSubSection(const std::string& name)
{
    if (name == "then") 
    {
        FillThenSection = false;
        return false;
    } else if (name == "else") 
    {
        FillElseSection = false;
        return false;
    }
    else
    {
        if (name=="if") return true;
        else throw Exception("Action 'if': unrecognized final tag");
    }
    
}


bool ActionIf::CompareResultToBool(IfOperations op,int result)
{
    return ((((op == opGE)||(op == opGE_I))&&((result == 1)||(result == 0))) ||
            (((op == opLE)||(op == opLE_I))&&((result ==-1)||(result == 0))) ||
            (((op == opGT)||(op == opGT_I))&& (result == 1)) ||
            (((op == opLT)||(op == opLT_I))&& (result ==-1)) ||
            (((op == opEQ)||(op == opEQ_I))&& (result == 0)) ||
            (((op == opNE)||(op == opNE_I))&& (result != 0)));
}


bool ActionIf::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'if'...");

    bool isValidCondition = true;

    Property * property = context.getProperty(singleparam.Operand1);
    if (!property) return true;

    smsc_log_debug(logger,"Testing "+property->getStr()+" vs "+singleparam.Operand2);


    if ((singleparam.Operation == opUnknown) && singleparam.Operand2.empty()) 
    {
        isValidCondition = property->getBool();
    } 
    else
    {
        int result = 0;

        const char *name = 0;
        PropertyType pt;

        switch (singleparam.Operation)
        {
        case opEQ: 
        case opNE: 
        case opGT: 
        case opGE: 
        case opLT: 
        case opLE:
            pt = pt_str;
            break;

        default:
            pt = pt_int;
            break;
        }                
 
        if (ftUnknown == ActionContext::Separate(singleparam.Operand2,name)) 
        {
            if (pt == pt_str)
                result = property->Compare(singleparam.Operand2);
            else
                result = property->Compare(atoi(singleparam.Operand2.c_str())); 
        } 
        else
        {
            Property * valproperty = context.getProperty(singleparam.Operand2);
            if (valproperty) result = property->Compare(*valproperty,pt);
            else smsc_log_warn(logger,"Action 'if': Invalid property " + singleparam.Operand2);
        } 
    
        isValidCondition = CompareResultToBool(singleparam.Operation,result);
    }

    int key = 0;
    Action * value = 0;

    if (isValidCondition) 
    {
        smsc_log_debug(logger,"Action 'if': run 'then' section");
        for (IntHash<Action *>::Iterator it = ThenActions.First(); it.Next(key, value);)
        {
            if (!value->run(context)) return false;
        }
    } else
    {
        smsc_log_debug(logger,"Action 'if': run 'else' section");
        for (IntHash<Action *>::Iterator it = ElseActions.First(); it.Next(key, value);)
        {
            if (!value->run(context)) return false;
        }
    }
    return true;
}

}}}
