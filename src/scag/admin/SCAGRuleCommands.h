#ifndef _SCAG_RULE_COMMANDS_H_
#define _SCAG_RULE_COMMANDS_H_

#include "AdminCommand.h"
#include "scag/re/RuleEngine.h"


namespace scag { namespace admin {

class CommandRuleBase : public AdminCommand
{
protected:
    scag::re::RuleKey key;
    scag::transport::TransportType transport;

    std::string m_ProcessName;
    std::string strTransport;

    virtual void processRuleCommand() = 0;
public:
    CommandRuleBase(const CommandIds::IDS ID, const xercesc::DOMDocument * doc);
    virtual Response * CreateResponse(scag::Scag * SmscApp);
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

