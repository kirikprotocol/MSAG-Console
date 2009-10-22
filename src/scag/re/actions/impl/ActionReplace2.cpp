#include "ActionReplace2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/util/encodings/Encodings.h"

namespace scag2 {
namespace re {
namespace actions {

using scag::util::encodings::Convertor;

IParserHandler * ActionReplace::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'match': cannot have a child object");
}

bool ActionReplace::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionReplace::init(const SectionParams& params,PropertyObject propertyObject)
{
    bool bExist;
    ftVar = CheckParameter(params, propertyObject, "replace", "var", true, true, m_sVar, bExist);
    ftRegexp = CheckParameter(params, propertyObject, "replace", "regexp", true, true, m_sRegexp, bExist);
    ftValue = CheckParameter(params, propertyObject, "replace", "value", true, true, m_sValue, bExist);
    ftResult = CheckParameter(params, propertyObject, "replace", "result", true, false, m_sResult, bExist);

    if(ftVar == ftUnknown) 
        Convertor::UTF8ToUCS2(m_sVar.c_str(), m_sVar.size(), m_wstrVar);

    if (ftValue == ftUnknown) 
        Convertor::UTF8ToUCS2(m_sValue.c_str(), m_sValue.size(), m_wstrReplace);
    
    smsc_log_debug(logger,"Action 'replace':: init");
}

bool ActionReplace::getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val)
{
    Property * p = NULL;
    if (!(p = context.getProperty(str)))
    {
        smsc_log_error(logger,"Action 'smpp:receipt': invalid '%s' property '%s'", field_name, str.c_str());
        return false;
    }
    val.assign(p->getStr().c_str(),p->getStr().size());
    return true;
}

bool ActionReplace::run(ActionContext& context)
{
    std::string var, rep;

    smsc_log_debug(logger,"Run Action 'replace'...");

    RegExp re;
    std::string regexp, temp;

    if(ftRegexp == ftUnknown) 
        temp = m_sRegexp;
    else if(!getStrProperty(context, m_sRegexp, "regexp", temp))
        return true;
 
    Convertor::UTF8ToUCS2(temp.c_str(), temp.size(), regexp);

    char endbuff[2] = {0,0};
    regexp.append(endbuff,2);
    
    if(!re.Compile((uint16_t*)regexp.data(), (OP_OPTIMIZE|OP_STRICT)|((regexp[0] == '/') ? OP_PERLSTYLE:OP_SINGLELINE)))
    {
        smsc_log_warn(logger, "Action 'replace' Failed to compile regexp '%s'", temp.c_str());
        return true;
    }

    smsc_log_debug(logger, "Action 'replace' Regexp '%s' compiled", temp.c_str());

    std::string tvar;
    const char * pvar = m_sVar.c_str();
    if(ftVar != ftUnknown) 
    {
        if(!getStrProperty(context, m_sVar, "var", tvar))
            return true;
        smsc_log_debug(logger,"Action 'replace': var=%s", tvar.c_str());
        Convertor::UTF8ToUCS2(tvar.c_str(), tvar.size(), var);
        pvar = tvar.c_str();
    }
    else
        var = m_wstrVar;

    if(ftValue != ftUnknown) 
    {
        std::string temp2;
        if(!getStrProperty(context, m_sValue, "value", temp2))
            return true;
        smsc_log_debug(logger,"Action 'replace': value=%s", temp2.c_str());
        Convertor::UTF8ToUCS2(temp2.c_str(), temp2.size(), rep);
    }
    else
        rep = m_wstrReplace;

    var.append(endbuff,2);

    SMatch m[100];
    int n = 100;
    size_t pos = 0;
    size_t prevpos;
    std::string result;

    while(re.Search((uint16_t*)(var.data() + pos), m, n))
    {
        smsc_log_debug(logger,"Action 'replace': match: pos=%d", m[0].start);    
        result.append(var.data() + pos, m[0].start * 2);
        result.append(rep.data(), rep.size());
        prevpos = pos;
        pos += 2 * m[0].end;
        if ( pos == prevpos ) {
            smsc_log_warn(logger,"Action 'replace': neverending loop, pos=%u!", unsigned(pos));
            break;
        }
        n = 100;
    }

    Property *p = context.getProperty(m_sResult);
    if(result.size() > 0)
    {
        if ( pos < var.size() - 2)
            result.append(var.data() + pos, var.size() - 2 - pos);
        std::string temp2;
        Convertor::UCS2ToUTF8((uint16_t*)result.data(), result.size() / 2, temp2);
        p->setStr(Property::string_type(temp2.c_str(),temp2.size()));
    }
    else
        p->setStr(pvar);

    smsc_log_debug(logger,"Action 'replace': result '%s'", p->getStr().c_str());
    return true;
}

}}}
