#ifndef __SCAG_RULE_ENGINE_ACTION_SEND__
#define __SCAG_RULE_ENGINE_ACTION_SEND__

#include "ActionContext.h"
#include "Action.h"
#include "sms/sms.h"
#include "core/buffers/Array.hpp"

namespace scag { namespace re { namespace actions {

using smsc::sms::Address;
using smsc::core::buffers::Array;

class ActionSend : public Action
{
public:
    enum SendLevel{Info = 0, Notify = 1, Warning = 2, Alarm = 3};

    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionSend (SendLevel l){ level = l; }

    virtual ~ActionSend();
    Logger * logger;

protected:
    ActionSend(const ActionSend&);

    std::string strDate, strDestPort, strEsmClass, strSrcPort;
    std::string strMsg;
    Array<std::string> toSms, toEmail;
    Array<std::string> fromSms, fromEmail;
    uint16_t destPort, srcPort;
    uint8_t esmClass;
    bool terminal, usr;
    FieldType ftDestPort, ftEsmClass, ftSrcPort;

    SendLevel level;

    PropertyObject propertyObject;

    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
    bool getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val);
};


}}}

#endif
