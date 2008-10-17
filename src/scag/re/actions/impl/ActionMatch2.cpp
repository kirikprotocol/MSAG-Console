#include "ActionMatch2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/util/encodings/Encodings.h"

namespace scag2 {
namespace re {
namespace actions {

using scag::util::encodings::Convertor;

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
    std::string temp;
    bool bExist;

    ftRegexp = CheckParameter(params, propertyObject, "match", "regexp", true, true, strRegexp, bExist);
    if(ftRegexp == ftUnknown)
        Convertor::UTF8ToUCS2(strRegexp.c_str(),strRegexp.size(),wstrRegexp);

    m_ftValue = CheckParameter(params, propertyObject, "match", "value", true, true, strValue, bExist);
    if(m_ftValue == ftUnknown)
        Convertor::UTF8ToUCS2(strValue.c_str(),strValue.size(),wstrValue);

    CheckParameter(params, propertyObject, "match", "result", true, false, strResult, bExist);

    smsc_log_debug(logger,"Action 'match':: init");
}

bool ActionMatch::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'match'...");

    Property * pValue = 0;
    Property *pResult = context.getProperty(strResult);
    if (!pResult) 
    {
        smsc_log_warn(logger,"Action 'match': invalid property '%s'", strResult.c_str());
        return true;
    }
    pResult->setBool(false);

    std::string regexp;
    if(ftRegexp != ftUnknown)
    {
        Property * pRegexp = context.getProperty(strRegexp);
        if (!pRegexp)
        {
            smsc_log_warn(logger, "Action 'match': invalid property '%s'", strRegexp.c_str());
            return true;
        }

        const Property::string_type& temp = pRegexp->getStr();
        Convertor::UTF8ToUCS2(temp.c_str(), temp.size(), regexp);
    }
    else
        regexp = wstrRegexp;

    char endbuff[2] = {0,0};
    regexp.append(endbuff,2);

    RegExp re;

    if(!re.Compile((uint16_t*)regexp.data(), (OP_OPTIMIZE|OP_STRICT)|((strRegexp[0] == '/') ? OP_PERLSTYLE:OP_SINGLELINE)))
    {
        smsc_log_warn(logger, "Action 'match' Failed to compile regexp '%s'", strRegexp.c_str());
        return true;
    }

    std::string value;
    if (m_ftValue != ftUnknown) 
    {
        pValue = context.getProperty(strValue);
        if (!pValue) 
        {
            smsc_log_warn(logger, "Action 'match': invalid property '%s'", strValue.c_str());
            return true;
        }
        const Property::string_type& temp = pValue->getStr();
        Convertor::UTF8ToUCS2(temp.c_str(), temp.size(), value);
    }
    else
        value = wstrValue;

    value.append(endbuff, 2);

    smsc_log_debug(logger,"Action 'match': regexp '%s', value '%s'", strRegexp.c_str(), strValue.c_str());

    SMatch m[100];
    int n=100;

    bool flag = re.Match((uint16_t*)value.data(),m,n);

    pResult->setBool(flag);
    smsc_log_debug(logger,"Action 'match': result '%d'", flag);

    return true;
}

}}}
