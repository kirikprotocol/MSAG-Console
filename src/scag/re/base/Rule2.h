#ifndef __SCAG_RULE_H__
#define __SCAG_RULE_H__

#include "core/buffers/IntHash.hpp"
// #include "scag/sessions/base/Session2.h"
// #include "EventHandler2.h"
#include "IParserHandler2.h"
#include "RuleStatus2.h"
#include "RuleKey2.h"

namespace scag2 {

namespace sessions {
class Session;
}

namespace transport {
class SCAGCommand;
}

namespace util {
class HRTiming;
}


namespace re {

using smsc::core::buffers::IntHash;
using namespace transport;
using smsc::logger::Logger;

class EventHandler;

namespace actions {
struct CommandProperty;
}

class Rule : public IParserHandler
{
    Rule(const Rule &);
    IntHash <EventHandler *> Handlers;
    smsc::core::synchronization::Mutex ruleLock;
    int useCounter;
    TransportType transportType;
    Logger * logger;

    EventHandler * CreateEventHandler();
//////////////IParserHandler Interfase///////////////////////
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////
public:
    TransportType getTransportType() const {return transportType;};

    void ref() {
        smsc::core::synchronization::MutexGuard mg(ruleLock);
        useCounter++;
    }
    void unref() 
    {
        bool del = false;
        {
            smsc::core::synchronization::MutexGuard mg(ruleLock);
            del = (--useCounter == 0);
        }
        if (del) delete this;
    }

    virtual void init(const SectionParams& params, PropertyObject propertyObject);


    /** 
     * Processes command via one of rules's handler.
     * Searches handler by command id/type and
     * returns RuleStatus after handler execution.
     * @param   command     command to process
     * @return  status      rule's handler execution status
     */
    virtual void process( SCAGCommand& command, sessions::Session& session,
                          RuleStatus& rs, actions::CommandProperty& cp, util::HRTiming* hrt = 0 );
    virtual void processSession( sessions::Session& session,
                                 RuleStatus& rs,
                                 const RuleKey& rk );

    Rule(): useCounter(1), transportType(SMPP),logger(0) {logger = Logger::getInstance("scag.re");};
    virtual ~Rule();
};

}
}

#endif
