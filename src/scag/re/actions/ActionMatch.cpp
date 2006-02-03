#include "ActionMatch.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {


IParserHandler * ActionMatch::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'match': cannot have a child object");
}

bool ActionMatch::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionMatch::init(const SectionParams& params,PropertyObject propertyObject)
{
    if (re) {
        delete re;
        re = 0;
    }

    logger = Logger::getInstance("scag.re");


    if (!params.Exists("regexp")) throw SCAGException("Action 'match': missing 'regexp' parameter");
    if (!params.Exists("value")) throw SCAGException("Action 'match': missing 'value' parameter");
    if (!params.Exists("result")) throw SCAGException("Action 'match': missing 'result' parameter");


    const char * name = 0;

    w_Regexp = params["regexp"];

    w_Value = params["value"];

    w_Result = params["result"];
    s_Result = ConvertWStrToStr(w_Result);


    AccessType at;

    ftValue = ActionContext::Separate(s_Value,name); 

    if (ftValue == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'match': cannot read property '%s' - no access", s_Value.c_str());
    }

    FieldType ftResult = ActionContext::Separate(s_Result,name); 

    if (ftResult == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atWrite)) 
            throw InvalidPropertyException("Action 'match': cannot read property '%s' - no access",s_Result.c_str());
    }

    if (ftResult == ftUnknown) throw InvalidPropertyException("Action 'match': unrecognized variable prefix '%s' for 'result' parameter",FormatWStr(w_Result).c_str());


    re = new RegExp();

    if(!re->Compile((char *)w_Regexp.c_str(),OP_OPTIMIZE|OP_STRICT))
    {
        throw SCAGException("Action 'match' Failed to compile regexp");
        //smsc_log_error(logger, "Action 'match' Failed to compile regexp");
    }

    smsc_log_debug(logger,"Action 'match':: init...");
}

bool ActionMatch::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'match'...");

    Property * pValue = 0;
    Property * pResult = 0;

    std::string value,s2;

    if (ftValue != ftUnknown) 
    {
        pValue = context.getProperty(s_Value);
        if (!pValue) 
        {
            smsc_log_warn(logger,"Action 'match': invalid property '%s'", s_Value.c_str());
            return true;
        }
        std::wstring wstr = pValue->getStr();

        value = FormatWStr(wstr);
    } else value = FormatWStr(w_Value);

    pResult = context.getProperty(s_Result);

    if (!pResult) 
    {
        smsc_log_warn(logger,"Action 'match': invalid property '%s'", s_Result.c_str());
        return true;
    }

  SMatch m[10];
  int n=10;
  bool flag = re->Match(value.c_str(),m,n);

  pResult->setBool(flag);

  return true;
}

ActionMatch::ActionMatch() : re(0)
{
}



ActionMatch::~ActionMatch() 
{
    if (re) delete re;
    //smsc_log_debug(logger, "'match' action released");
}



}}}