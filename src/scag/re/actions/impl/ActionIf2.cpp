#include "ActionIf2.h"

#include "scag/re/base/ActionFactory2.h"
#include "scag/re/base/CommandAdapter2.h"


namespace scag2 {
namespace re {
namespace actions {


ActionIf::ActionIf() : FillThenSection(true), FillElseSection(false)
{
}

const char* ActionIf::getStrFromOperation( IfOperations op ) const
{
    switch (op) {
    case (opEQ) : return "eq";
    case (opNE) : return "ne";
    case (opGT) : return "gt";
    case (opGE) : return "ge";
    case (opLT) : return "lt";
    case (opLE) : return "le";
    case (opEQ_I) : return "eqi";
    case (opGE_I) : return "gei";
    case (opLE_I) : return "lei";
    case (opNE_I) : return "nei";
    case (opGT_I) : return "gti";
    case (opLT_I) : return "lti";
    default: return "???";
    };
}


void ActionIf::init(const SectionParams& params,PropertyObject _propertyObject)
{
    propertyObject = _propertyObject;

    std::string temp;
    bool bExist;
    FieldType ft;

    ft = CheckParameter(params, propertyObject, "if", "test", true, true, temp, bExist);
    if (ft == ftUnknown) throw SCAGException("Action 'if': unrecognized variable prefix '%s' for 'test' parameter", singleparam.strOperand1.c_str());
    singleparam.strOperand1 = temp.c_str();

    m_hasOP = params.Exists("op");
    ftSecondOperandFieldType = CheckParameter(params, propertyObject, "if", "value", false, true, temp, bExist);
    if (m_hasOP&&(!bExist)) throw SCAGException("Action 'if': missing 'value' parameter"); 
    if ((!m_hasOP)&&bExist) throw SCAGException("Action 'if': missing 'op' parameter"); 
    singleparam.strOperand2 = temp.c_str();

    if (m_hasOP) singleparam.Operation = GetOperationFromSTR(params["op"]);

    smsc_log_debug(logger,"Action 'if':: init");
}

ActionIf::~ActionIf()
{
    // int key;
    // Action * value;

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
            smsc_log_debug(logger,"Action 'if': child object '%s' set to 'then'", name.c_str());
        }
        else if (FillElseSection) 
        {
            ElseActions.push_back(action);
            smsc_log_debug(logger,"Action 'if': child object '%s' set to 'else'", name.c_str());
        }

        return action;
    }
}

bool ActionIf::FinishXMLSubSection(const std::string& name)
{

    const size_t thenPos = name.find("then");
    const size_t endPos = name.find("else");

    //smsc_log_debug(logger, "Action <if>: --- section %s, %d, %d", name.c_str(), thenPos, endPos);

    if (thenPos != std::string::npos)
    {
        //smsc_log_debug(logger, "Action <if>: <then> closed");

        FillThenSection = false;
        return false;
    }
    else if (endPos != std::string::npos)
    {
        //smsc_log_debug(logger, "Action <if>: <else> closed");

        FillElseSection = false;
        return false;
    }
/*#ifndef NDEBUG
    else if (name != "if") throw SCAGException("Action 'if': unrecognized final tag '%s'", name.c_str());
#endif
  */
    smsc_log_debug(logger, "Action <if>: action closed" );
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

    do {

        if ( ! longCallContext.ActionStack.empty() ) {
            isTrueCondition = longCallContext.ActionStack.top().thenSection;
            break;
        }

        Property * property = context.getProperty(singleparam.strOperand1.c_str());
        if (!property) 
        {
            smsc_log_debug(logger,"Action 'If' cannot find property '%s'", singleparam.strOperand1.c_str());
            isTrueCondition = false;
            break;
        }

        if (!m_hasOP) 
        {
            smsc_log_debug(logger,"Testing %s = '%lld' for bool", singleparam.strOperand1.c_str(), property->getInt());
            isTrueCondition = 
                ( property->getType() == pt_str ?
                  ! property->getStr().empty() :
                  bool(property->getInt()) );
            break;
        } 


        //std::string str = property->getStr();
        //smsc_log_debug(logger,"Testing %s='%s' vs %s",singleparam.strOperand1.c_str(),FormatWStr(str).c_str(),ConvertWStrToStr(singleparam.wstrOperand2).c_str());

        smsc_log_debug(logger,"Testing %s = '%s' %s '%s'",
                       singleparam.strOperand1.c_str(),
                       property->getStr().c_str(),
                       getStrFromOperation(singleparam.Operation),
                       singleparam.strOperand2.c_str());

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
            Property * valproperty = context.getProperty(singleparam.strOperand2.c_str());
            if (valproperty) result = property->Compare(*valproperty,pt);
            else smsc_log_warn(logger,"Action 'if': Invalid property '%s'", singleparam.strOperand2.c_str());
        }
        isTrueCondition = CompareResultToBool(singleparam.Operation,result);

    } while ( false ); // fake loop

    smsc_log_debug(logger,"Action 'if': run '%s' section", isTrueCondition ? "then" : "else");
    bool b =  RunActionVector(context, longCallContext, isTrueCondition ? ThenActions : ElseActions, logger);
    context.isTrueCondition = isTrueCondition;
    return b;
}

}}}
