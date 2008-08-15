#ifndef _SCAG_RULE_COMMANDS2_H_
#define _SCAG_RULE_COMMANDS2_H_

#include "AdminCommand2.h"
#include "scag/re/base/RuleKey2.h"


namespace scag2 {
namespace admin {

class CommandRuleBase : public AdminCommand
{
protected:
    re::RuleKey              key;
    transport::TransportType transport;

    std::string m_ProcessName;
    std::string strTransport;

    virtual void processRuleCommand() = 0;
public:
    CommandRuleBase(const CommandIds::IDS ID, const xercesc::DOMDocument * doc);
    virtual Response * CreateResponse(Scag * SmscApp);
    virtual void init();
};


class CommandAddRule : public CommandRuleBase
{
protected:
    virtual void processRuleCommand();
public:
  
  CommandAddRule(const xercesc::DOMDocument * doc);
};

class CommandUpdateRule : public CommandRuleBase
{
protected:
    virtual void processRuleCommand();
public:
  CommandUpdateRule(const xercesc::DOMDocument * doc);
};

class CommandRemoveRule : public CommandRuleBase
{
protected:
    virtual void processRuleCommand();
public:
  CommandRemoveRule(const xercesc::DOMDocument * doc);
};

}}

#endif

