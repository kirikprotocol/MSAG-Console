#include "ActionTrafficCheck.h"
#include "scag/re/actions/ActionFactory.h"
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
        throw SCAGException("Action 'traffic:check': unrecognized child object '%s' to create",name.c_str());

    try
    {
        action->init(params,propertyObject);
    } catch (SCAGException& e)
    {
        delete action;
        throw;
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
    propertyObject = _propertyObject;

    FieldType ft;
    bool bExist;
    std::string sPeriod;

    m_ftMax = CheckParameter(params, propertyObject, "traffic:check", "max", true, true, m_sMax, bExist);

    ft = CheckParameter(params, propertyObject, "traffic:check", "period", true, true, sPeriod, bExist);

    if (!StrToPeriod(m_period,sPeriod)) SCAGException("Action 'traffic:check': invalid value '%s' for 'period' parameter", sPeriod.c_str());
}

bool ActionTrafficCheck::run(ActionContext& context)
{
    int key;
    Action * value;

    std::list<Action *>::const_iterator it;

    
    std::string routeId;
    int nMaxValue = 0;

    if (m_ftMax == ftUnknown) 
        nMaxValue = atoi(m_sMax.c_str());
    else
    {
        Property * property = context.getProperty(m_sMax);

        if (property) 
            nMaxValue = property->getInt();
        else 
            smsc_log_warn(logger,"Action 'traffic:check': invalid property");
    }


    if (!Statistics::Instance().checkTraffic(routeId, m_period, nMaxValue)) return true;

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
}

}}}
