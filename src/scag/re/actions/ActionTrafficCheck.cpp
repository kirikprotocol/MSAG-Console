#include "ActionTrafficCheck.h"
#include "ActionFactory.h"
#include "scag/re/SAX2Print.hpp"

#include "scag/stat/Statistics.h"



namespace scag { namespace re { namespace actions {

bool ActionTrafficCheck::StrToPeriod(CheckTrafficPeriod& _period, std::string& str)
{
    if (str == "min") 
    {
        _period = checkMinPeriod;
        return true;
    } else if (str == "hour") 
    {
        _period = checkHourPeriod;
        return true;
    } else if (str == "day") 
    {
        _period = checkDayPeriod;
        return true;
    } else if (str == "month") 
    {
        _period = checkMonthPeriod;
        return true;
    }

    return false;
}



IParserHandler * ActionTrafficCheck::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    Action * action = 0;
    action = factory.CreateAction(name);
    if (!action) 
        throw RuleEngineException("Action 'traffic:check': unrecognized child object '",name.c_str(),"' to create");

    try
    {
        action->init(params,propertyObject);
    } catch (Exception& e)
    {
        delete action;
        throw e;
    }
    Actions.push_back(action);
    return action;
}


bool ActionTrafficCheck::FinishXMLSubSection(const std::string& name)
{
    return (name == "traffic:check");
}

void ActionTrafficCheck::init(const SectionParams& params, PropertyObject _propertyObject)
{
    if (!params.Exists("max")) throw RuleEngineException("Action 'traffic:check': missing 'max' parameter");
    if (!params.Exists("period")) throw RuleEngineException("Action 'traffic:check': missing 'period' parameter");

    sMax = params["max"];

    std::string sPeriod = params["period"];
    if (!StrToPeriod(period,sPeriod)) RuleEngineException("Action 'traffic:check': invalid value '",sPeriod.c_str(),"' for 'period' parameter");
    propertyObject = _propertyObject;
}

bool ActionTrafficCheck::run(ActionContext& context)
{
    int key;
    Action * value;

    std::list<Action *>::const_iterator it;

    
    std::string routeId;
    int nMaxValue = 0;
    const char * name;
    
    FieldType ft;
    ft = ActionContext::Separate(sMax,name);

    if (ft == ftUnknown) 
        nMaxValue = atoi(sMax.c_str());
    else
    {
        Property * p = context.getProperty(sMax);

        if (p) 
            nMaxValue = p->getInt();
        else 
            smsc_log_warn(logger,"Action 'traffic:check': invalid property");
    }


    if (!context.checkTraffic(routeId,period,nMaxValue)) return true;

    for (it = Actions.begin(); it!=Actions.end(); ++it)
    {
        if (!(*it)->run(context)) return false;
    }
    return true;
}

ActionTrafficCheck::~ActionTrafficCheck()
{
    int key;
    Action * value;

    std::list<Action *>::const_iterator it;

    for (it = Actions.begin(); it!=Actions.end(); ++it)
    {
        delete (*it);
    }

    smsc_log_debug(logger, "'TrafficCheck' action released");
}


}}}