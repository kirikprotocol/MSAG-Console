#ifndef _SCAG_RULE_COMMANDS_H_
#define _SCAG_RULE_COMMANDS_H_

#include "SCAGCommand.h"
#include "scag/re/RuleEngine.h"


namespace scag { namespace admin {

class CommandRuleBase : public scag::admin::SCAGCommand
{
protected:
    scag::re::RuleKey key;
    scag::transport::TransportType transport;
    bool hasErrors;
    std::string m_ProcessName;
    std::string strTransport;

    bool readParams(const xercesc::DOMDocument * document);
    virtual void processRuleCommand() = 0;
public:
    CommandRuleBase(const CommandIds::IDS ID, const xercesc::DOMDocument * doc);
    virtual Response * CreateResponse(scag::Scag * SmscApp);
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

