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
    logger = Logger::getInstance("scag.re");

    FieldType ft;
    std::string temp;
    bool bExist;

    ft = CheckParameter(params, propertyObject, "match", "regexp", true, true, wstrRegexp, bExist);
    if (ft!=ftUnknown) throw SCAGException("Action 'match': 'regexp' parameter must be a scalar constant type");

    m_ftValue = CheckParameter(params, propertyObject, "match", "value", true, true, wstrValue, bExist);
    strValue = ConvertWStrToStr(wstrValue);

    ft = CheckParameter(params, propertyObject, "match", "result", true, false, temp, bExist);
    strResult = ConvertWStrToStr(temp);

    re = new RegExp();

    temp = "";
    temp.append(wstrRegexp.data(), wstrRegexp.size());
    char endbuff[2] = {0,0};
    temp.append(endbuff,2);

    if (!re->Compile((unsigned short *)temp.data(), OP_OPTIMIZE|OP_STRICT))
        throw SCAGException("Action 'match' Failed to compile regexp");

    smsc_log_debug(logger,"Action 'match':: init");
}

bool ActionMatch::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'match'...");


    Property * pValue = 0;
    Property * pResult = 0;

    std::string value,s2;

    if (m_ftValue != ftUnknown) 
    {
        pValue = context.getProperty(strValue);
        if (!pValue) 
        {
            smsc_log_warn(logger,"Action 'match': invalid property '%s'", strValue.c_str());
            return true;
        }
        value = pValue->getStr();
    } else value = wstrValue;

    smsc_log_warn(logger,"Action 'match': regexp '%s', value '%s'", FormatWStr(wstrRegexp).c_str(),FormatWStr(value).c_str());


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
  smsc_log_warn(logger,"Action 'match': result '%d'", flag);

  return true;
}

ActionMatch::ActionMatch() : re(0)
{
}



ActionMatch::~ActionMatch() 
{
    if (re) delete re;
}



}}}