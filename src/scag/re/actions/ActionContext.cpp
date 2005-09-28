#include "ActionContext.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag { namespace re { namespace actions 
{
    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::util::properties;
    using namespace scag::exceptions;

void ActionContext::AddPendingOperation(uint8_t type, time_t pendingTime)
{
    PendingOperation pendingOperation;
    pendingOperation.validityTime = pendingTime;
    pendingOperation.type = type;
    
    session.addPendingOperation(pendingOperation);
}


FieldType ActionContext::Separate(const std::string& var,const char *& name)
{
    if (var.size()<=1) return ftUnknown;
    
    name = var.c_str() + 1;

    if (var[0] == ftLocal) return ftLocal;
    if (var[0] == ftConst) return ftConst;
    if (var[0] == ftField) return ftField;
    if (var[0] == ftSession) return ftSession;

    return ftUnknown;
}


Property* ActionContext::getProperty(const std::string& var)
{
    FieldType prefix;
    const char * name;
    Property * p = 0;

    prefix = Separate(var,name);

    switch (prefix) 
    {
    case ftLocal:
        if (!variables.Exists(name)) 
        {
            Property property;
            variables.Insert(name,property);
        }
        return variables.GetPtr(name);
        break;

    case ftConst:
        return constants.GetPtr(name);
        break;

    case ftField: return command.getProperty(name);
    case ftSession: return session.getProperty(name);

    default:
        return 0;
    }

}

void ActionContext::abortSession()
{
    session.abort();
}


bool ActionContext::checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value)
{
    return statistics.checkTraffic(routeId, period, value);
}

BillKey ActionContext::CreateBillKey()
{
    BillKey billKey;
    Property * property1 = 0;
    Property * property2 = 0;

    property1 = command.getProperty("OA");
    property2 = command.getProperty("DA");

    if ((!property1)||(property2)) throw SCAGException("ActionContext error : cannto create BillKey");

    billKey.DA = property2->getStr();
    billKey.OA = property1->getStr();
    return billKey;
}



}}}

