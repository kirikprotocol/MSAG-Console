#include "ActionMatch.h"
#include "scag/re/CommandAdapter.h"
#include "scag/util/encodings/Encodings.h"

namespace scag { namespace re { namespace actions {

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
    logger = Logger::getInstance("scag.re");

    FieldType ft, ftRegexp;
    std::string temp;
    bool bExist;

    ftRegexp = CheckParameter(params, propertyObject, "match", "regexp", true, true, strRegexp, bExist);
    //if (ft!=ftUnknown) throw SCAGException("Action 'match': 'regexp' parameter must be a scalar constant type");
    Convertor::UTF8ToUCS2(strRegexp.c_str(),strRegexp.size(),wstrRegexp);

    m_ftValue = CheckParameter(params, propertyObject, "match", "value", true, true, strValue, bExist);
    Convertor::UTF8ToUCS2(strValue.c_str(),strValue.size(),wstrValue);

    ft = CheckParameter(params, propertyObject, "match", "result", true, false, strResult, bExist);

    re = new RegExp();

    if (ftRegexp == ftUnknown) 
    {
        temp = "";
        temp.append(wstrRegexp.data(), wstrRegexp.size());
        char endbuff[2] = {0,0};
        temp.append(endbuff,2);


        if (!re->Compile((unsigned short *)temp.data(), OP_OPTIMIZE|OP_STRICT))
            throw SCAGException("Action 'match' Failed to compile regexp");

        m_Compiled = true;
    } else m_Compiled = false;

    smsc_log_debug(logger,"Action 'match':: init");
}

bool ActionMatch::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'match'...");


    Property * pValue = 0;
    Property * pResult = 0;

    if (!m_Compiled) 
    {
        Property * pRegexp = context.getProperty(strRegexp);
        if (!pRegexp)
        {
            smsc_log_warn(logger, "Action 'match': invalid property '%s'", strRegexp.c_str());

            pResult = context.getProperty(strResult);
            if (!pResult) 
            {
                smsc_log_warn(logger,"Action 'match': invalid property '%s'", strResult.c_str());
                return true;
            }

            pResult->setBool(false);
            return true;
        }

        std::string temp = pRegexp->getStr();
        std::string regexp;
        Convertor::UTF8ToUCS2(temp.c_str(),temp.size(),regexp);


        char endbuff[2] = {0,0};
        regexp.append(endbuff,2);

        if (!re->Compile((unsigned short *)regexp.data(), OP_OPTIMIZE|OP_STRICT))
        {
            smsc_log_warn(logger, "Action 'match' Failed to compile regexp '%s'",temp.c_str());

            pResult = context.getProperty(strResult);
            if (!pResult) 
            {
                smsc_log_warn(logger,"Action 'match': invalid property '%s'", strResult.c_str());
                return true;
            }

            pResult->setBool(false);

            return true;
        }
        smsc_log_debug(logger, "Action 'match' Regexp '%s' compiled", temp.c_str());
    }

    std::string value,s2;

    if (m_ftValue != ftUnknown) 
    {
        pValue = context.getProperty(strValue);
        if (!pValue) 
        {
            smsc_log_warn(logger, "Action 'match': invalid property '%s'", strValue.c_str());
            return true;
        }
        std::string temp = pValue->getStr();
        Convertor::UTF8ToUCS2(temp.c_str(),temp.size(), value);
    } else value = wstrValue;

    smsc_log_warn(logger,"Action 'match': regexp '%s', value '%s'", strRegexp.c_str(),strValue.c_str());


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