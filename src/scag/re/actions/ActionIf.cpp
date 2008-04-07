#include "ActionIf.h"

#include "ActionFactory.h"
#include "scag/re/CommandAdapter.h"


namespace scag { namespace re { namespace actions {


ActionIf::ActionIf() : FillThenSection(true), FillElseSection(false)
{
}

void ActionIf::init(const SectionParams& params,PropertyObject _propertyObject)
{
    propertyObject = _propertyObject;

    std::string temp;
    bool bExist;
    FieldType ft;

    ft = CheckParameter(params, propertyObject, "if", "test", true, true, singleparam.strOperand1, bExist);

    if (ft == ftUnknown) throw SCAGException("Action 'if': unrecognized variable prefix '%s' for 'test' parameter", singleparam.strOperand1.c_str());

    m_hasOP = params.Exists("op");
    ftSecondOperandFieldType = CheckParameter(params, propertyObject, "if", "value", false, true, singleparam.strOperand2, bExist);

    if (m_hasOP&&(!bExist)) throw SCAGException("Action 'if': missing 'value' parameter"); 
    if ((!m_hasOP)&&bExist) throw SCAGException("Action 'if': missing 'op' parameter"); 

    if (m_hasOP) singleparam.Operation = GetOperationFromSTR(params["op"]);

    smsc_log_debug(logger,"Action 'if':: init");
}

ActionIf::~ActionIf()
{
    int key;
    Action * value;

    std::vector<Action *>::const_iterator it;

    for (it = ThenActions.begin(); it!=ThenActions.end(); ++it)
    {
        delete (*it);
    }

    for (it = ElseActions.begin(); it!=ElseActions.end(); ++it)
    {
        delete (*it);
    }

//    smsc_log_debug(logger, "'if' action released");
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
            throw SCAGException("Action 'if': unrecognized child object '%s' to create",name.c_str());

        try
        {
            action->init(params,propertyObject);
        } catch (SCAGException& e)
        {
            delete action;
            throw;
        }

        if (FillThenSection) 
        {
            ThenActions.push_back(action);
            smsc_log_debug(logger,"Action 'if': child object set to 'then'");
        }
        else if (FillElseSection) 
        {
            ElseActions.push_back(action);
            smsc_log_debug(logger,"Action 'if': child object set to 'else'");
        }

        return action;
    }
}

bool ActionIf::FinishXMLSubSection(const std::string& name)
{

    int thenPos = name.find("then", -1);
    int endPos = name.find("else", -1);

    //smsc_log_debug(logger, "Action <if>: --- section %s, %d, %d", name.c_str(), thenPos, endPos);

    if (thenPos >= 0) 
    {
        //smsc_log_debug(logger, "Action <if>: <then> closed");

        FillThenSection = false;
        return false;
    }
    else if (endPos >= 0) 
    {
        //smsc_log_debug(logger, "Action <if>: <else> closed");

        FillElseSection = false;
        return false;
    }
/*#ifndef NDEBUG
    else if (name != "if") throw SCAGException("Action 'if': unrecognized final tag '%s'", name.c_str());
#endif
  */
    smsc_log_debug(logger, "Action <if>: action closed");
    return true;
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
    bool isTrueCondition;
    smsc_log_debug(logger,"Run Action 'if'...");

    LongCallContext &longCallContext = context.getSession().getLongCallContext();

    if (longCallContext.ActionStack.empty()) 
    {
        Property * property = context.getProperty(singleparam.strOperand1);
        if (!property) 
        {
            smsc_log_debug(logger,"Action 'If' stopped. Details: Cannot find property '%s'", singleparam.strOperand1.c_str());
            return true;
        }

        if (!m_hasOP) 
        {
            smsc_log_debug(logger,"Testing %s = '%lld' for bool", singleparam.strOperand1.c_str(), property->getInt());
            isTrueCondition = property->getBool();
        } 
        else
        {
            //std::string str = property->getStr();
            //smsc_log_debug(logger,"Testing %s='%s' vs %s",singleparam.strOperand1.c_str(),FormatWStr(str).c_str(),ConvertWStrToStr(singleparam.wstrOperand2).c_str());

            smsc_log_debug(logger,"Testing %s = '%s' vs '%s'", singleparam.strOperand1.c_str(), property->getStr().c_str(),singleparam.strOperand2.c_str());

            int result = 0;

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

            if (ftUnknown == ftSecondOperandFieldType) 
            {
                if (pt == pt_str)
                    result = property->Compare(singleparam.strOperand2);
                else
                    result = property->Compare(atoi(singleparam.strOperand2.c_str())); 
            }
            else
            {
                Property * valproperty = context.getProperty(singleparam.strOperand2);
                if (valproperty) result = property->Compare(*valproperty,pt);
                else smsc_log_warn(logger,"Action 'if': Invalid property '%s'", singleparam.strOperand2.c_str());
            }

            isTrueCondition = CompareResultToBool(singleparam.Operation,result);
        }
    }
    else
        isTrueCondition = longCallContext.ActionStack.top().thenSection;

    smsc_log_debug(logger,"Action 'if': run '%s' section", isTrueCondition ? "then" : "else");
    bool b =  RunActionVector(context, longCallContext, isTrueCondition ? ThenActions : ElseActions, logger);
    context.isTrueCondition = isTrueCondition;
    return b;
}

}}}
