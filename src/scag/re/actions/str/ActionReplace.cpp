#include "ActionReplace.h"
#include "scag/re/CommandAdapter.h"
#include "scag/util/encodings/Encodings.h"

namespace scag { namespace re { namespace actions {

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
    paramVar = std::auto_ptr<ActionParameter>(new ActionParameter(params, propertyObject, "replace", "var", true, true, logger));
    paramRegexp = std::auto_ptr<ActionParameter>(new ActionParameter(params, propertyObject, "replace", "regexp", true, true, logger));
    paramValue = std::auto_ptr<ActionParameter>(new ActionParameter(params, propertyObject, "replace", "value", true, true, logger));
    paramResult = std::auto_ptr<ActionParameter>(new ActionParameter(params, propertyObject, "replace", "result", true, false, logger));

    re = new RegExp();

    if (!paramRegexp->isProperty()) 
    {
        std::string temp;

        Convertor::UTF8ToUCS2(paramRegexp->getStrValue().c_str(),paramRegexp->getStrValue().size(), temp);
        if (temp.size() == 0) throw SCAGException("Action 'replace': invalid parameter 'regexp'. Delails: Cannot use blank string.");

        char endbuff[2] = {0,0};
        temp.append(endbuff,2);

        //OP_SINGLELINE
        m_type = (OP_OPTIMIZE|OP_STRICT)|((temp[0] == '/') ? OP_PERLSTYLE:OP_SINGLELINE);
        //m_type = (OP_OPTIMIZE|OP_STRICT|OP_SINGLELINE);


        if (!re->Compile((unsigned short *)temp.data(), m_type))
            throw SCAGException("Action 'replace' Failed to compile regexp");
    }

    if (!paramVar->isProperty()) 
        Convertor::UTF8ToUCS2(paramValue->getStrValue().c_str(),paramValue->getStrValue().size(), m_wstrVar);

    if (!paramValue->isProperty()) 
        Convertor::UTF8ToUCS2(paramValue->getStrValue().c_str(),paramValue->getStrValue().size(), m_wstrReplace);
    
    smsc_log_debug(logger,"Action 'replace':: init");
}

bool ActionReplace::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'replace'...");

    if (!paramVar->prepareValue(context)) return true;
    if (!paramRegexp->prepareValue(context)) return true;
    if (!paramValue->prepareValue(context)) return true;
    if (!paramResult->prepareValue(context)) return true;

    if (paramRegexp->isProperty()) 
    {
        std::string regexp;
        Convertor::UTF8ToUCS2(paramRegexp->getStrValue().c_str(),paramRegexp->getStrValue().size(),regexp);

        char endbuff[2] = {0,0};
        regexp.append(endbuff,2);

        m_type = (OP_OPTIMIZE|OP_STRICT)|((regexp[0] == '/') ? OP_PERLSTYLE:OP_SINGLELINE);

        if (!re->Compile((unsigned short *)regexp.data(), m_type))
        {
            smsc_log_warn(logger, "Action 'replace' Failed to compile regexp '%s'", paramRegexp->getStrValue().c_str());
            return true;
        }

        smsc_log_debug(logger, "Action 'replace' Regexp '%s' compiled", paramRegexp->getStrValue().c_str());
    }

    if (paramVar->isProperty()) 
    {
        std::string temp = paramVar->getStrValue();
        smsc_log_debug(logger,"Action 'replace': var=%s", temp.c_str());
        Convertor::UTF8ToUCS2(temp.c_str(),temp.size(), m_wstrVar);
    }

    if (paramValue->isProperty()) 
    {
        std::string temp = paramValue->getStrValue();
        smsc_log_debug(logger,"Action 'replace': value=%s", temp.c_str());
        Convertor::UTF8ToUCS2(temp.c_str(),temp.size(), m_wstrReplace);
    }

    char endbuff[2] = {0,0};
    m_wstrVar.append(endbuff,2);

    SMatch m[100];
    int n = 100, pos = 0;
    std::string result;

    while(re->Search((uint16_t*)m_wstrVar.data() + pos, m, n))
    {
        result.append(m_wstrVar.data() + pos * 2, m[0].start * 2);
        result.append(m_wstrReplace.data(), m_wstrReplace.size());
        pos += m[0].end;
        n = 100;
    }

    if(result.size() > 0)
    {
        if(2 * pos < m_wstrVar.size() - 2)
            result.append(m_wstrVar.data() + 2 * pos, m_wstrVar.size() - 2 - 2 * pos);
        std::string temp;
        Convertor::UCS2ToUTF8((uint16_t*)result.data(), result.size() / 2, temp);
        paramResult->setStrValue(temp);
    }
    else
        paramResult->setStrValue(paramVar->getStrValue());        

    smsc_log_debug(logger,"Action 'replace': result '%s'", paramResult->getStrValue().c_str());
    return true;
}

ActionReplace::ActionReplace() : re(0)
{
}

ActionReplace::~ActionReplace() 
{
    if (re) delete re;
}

}}}
