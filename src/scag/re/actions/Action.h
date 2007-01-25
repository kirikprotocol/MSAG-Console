#ifndef __SCAG_RULE_ENGINE_ACTION__
#define __SCAG_RULE_ENGINE_ACTION__


#include "ActionContext.h"
#include <scag/re/actions/IParserHandler.h>
#include <logger/Logger.h>


namespace scag { namespace re { namespace actions 
{

using smsc::logger::Logger;

class ActionParameter
{
protected:
    std::string m_strVariable;
    Property * m_pProperty;

    FieldType m_FieldType;
    bool m_bReadOnly;
    bool m_bExists;
    Logger * m_pLogger;
    std::string m_actionName;
public:
    bool prepareValue(ActionContext& context);
    bool isReadOnly() {return m_bReadOnly;}
    bool Exists() {return m_bExists;}

    void setStrValue(std::string& value);
    void setIntValue(int value);

    std::string getStrValue();
    int getIntValue();
    bool isProperty()
    {
        return (m_FieldType != ftUnknown);
    }
    ActionParameter(const SectionParams& params, PropertyObject& propertyObject, const std::string& actionName, const char * paramName, bool isRequired, bool readOnly, Logger * logger);
};

class Action : public IParserHandler
{
protected:
    Logger * logger;
public:
    Action() : logger(0) {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject) = 0;
    virtual bool run(ActionContext& context) = 0;
    static FieldType CheckParameter(const SectionParams& params, PropertyObject& propertyObject, const char * actionName, const char * paramName, bool isRequired, bool readOnly, std::string& strParameter, bool& exist);
};


}}}

#endif // __SCAG_RULE_ENGINE_ACTION__

