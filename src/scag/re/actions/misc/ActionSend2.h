#ifndef __SCAG_RULE_ENGINE_ACTION_SEND2__
#define __SCAG_RULE_ENGINE_ACTION_SEND2__

#include "scag/re/actions/ActionContext2.h"
#include "scag/re/actions/Action2.h"
#include "sms/sms.h"
#include "core/buffers/Array.hpp"

namespace scag2 {
namespace re {
namespace actions {

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

protected:
    ActionSend(const ActionSend&);

    std::string strDate, strDestPort, strEsmClass, strSrcPort, strPacketType;
    std::string strMsg;
    Array<std::string> toSms, toEmail;
    Array<std::string> fromSms, fromEmail;
    uint16_t destPort, srcPort;
    uint8_t esmClass;
    bool terminal;
    FieldType ftDestPort, ftEsmClass, ftSrcPort, ftPacketType;

    SendLevel level;

    PropertyObject propertyObject;

    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
    bool getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val);
};


}}}

#endif
