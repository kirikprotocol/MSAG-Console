#ifndef __SCAG_RULE_H__
#define __SCAG_RULE_H__

#include "core/buffers/IntHash.hpp"
// #include "scag/sessions/base/Session2.h"
// #include "EventHandler2.h"
#include "IParserHandler2.h"
#include "RuleStatus2.h"
#include "RuleKey2.h"
#include "informer/io/EmbedRefPtr.h"

namespace eyeline {
namespace informer {
template < class T > class EmbedRefPtr;
}
}

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
    friend class eyeline::informer::EmbedRefPtr< Rule >;
public:
    TransportType getTransportType() const {return transportType;};

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

    Rule() :
    logger(smsc::logger::Logger::getInstance("scag.re")),
    ref_(0),
    transportType(SMPP) {}

    virtual ~Rule();

private:
    Rule(const Rule &);

    void ref() {
        smsc::core::synchronization::MutexGuard mg(refLock_);
        ++ref_;
    }

    void unref() 
    {
        {
            smsc::core::synchronization::MutexGuard mg(refLock_);
            if (--ref_) {
                return;
            }
        }
        delete this;
    }

private:
    Logger * logger;

    smsc::core::synchronization::Mutex refLock_;
    unsigned                           ref_;
    IntHash <EventHandler *> Handlers;
    TransportType transportType;

    EventHandler * CreateEventHandler();
//////////////IParserHandler Interfase///////////////////////
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////

};

typedef eyeline::informer::EmbedRefPtr< Rule > RulePtr;

}
}

#endif
