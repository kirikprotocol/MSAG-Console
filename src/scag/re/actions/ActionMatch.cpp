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

    wstrRegexp = params["regexp"];
    wstrValue = params["value"];

    strValue = ConvertWStrToStr(wstrValue);

    std::string wstrResult = params["result"];
    strResult = ConvertWStrToStr(wstrResult);


    AccessType at;

    ftValue = ActionContext::Separate(strValue,name); 

    if (ftValue == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'match': cannot read property '%s' - no access", FormatWStr(wstrValue).c_str());
    }

    FieldType ftResult = ActionContext::Separate(strResult,name); 

    if (ftResult == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atWrite)) 
            throw InvalidPropertyException("Action 'match': cannot read property '%s' - no access",strResult.c_str());
    }

    if (ftResult == ftUnknown) throw InvalidPropertyException("Action 'match': unrecognized variable prefix '%s' for 'result' parameter",FormatWStr(wstrResult).c_str());


    re = new RegExp();


    std::string regexpStr = ConvertWStrToStr(wstrRegexp);

    if(!re->Compile((unsigned short *)regexpStr.c_str(),OP_OPTIMIZE|OP_STRICT))
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
        pValue = context.getProperty(strValue);
        if (!pValue) 
        {
            smsc_log_warn(logger,"Action 'match': invalid property '%s'", strValue.c_str());
            return true;
        }
        value = pValue->getStr();
    } else value = wstrValue;

    pResult = context.getProperty(strResult);

    if (!pResult) 
    {
        smsc_log_warn(logger,"Action 'match': invalid property '%s'", strResult.c_str());
        return true;
    }

  SMatch m[10];
  int n=10;
  bool flag = re->Match((unsigned short *)value.data(),m,n);

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