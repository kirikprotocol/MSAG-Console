#ifndef __SCAG_RULE_H__
#define __SCAG_RULE_H__

#include <core/buffers/IntHash.hpp>
#include "scag/sessions/Session.h"
#include "EventHandler.h"
#include "RuleStatus.h"

namespace scag { namespace re 
{

using smsc::core::buffers::IntHash;
using scag::sessions::Session;
using namespace scag::transport;

class EventHandler;

class Rule : public IParserHandler
{
    int ExtractHandlerType(SCAGCommand& command);

    Rule(const Rule &);
    IntHash <EventHandler *> Handlers;
    Mutex ruleLock;
    int useCounter;
    TransportType transportType;

    EventHandler * CreateEventHandler();
//////////////IParserHandler Interfase///////////////////////
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////
public:
    TransportType getTransportType() const {return transportType;};

    void ref() {
        MutexGuard mg(ruleLock);
        useCounter++;
    }
    void unref() 
    {
        bool del = false;
        {
            MutexGuard mg(ruleLock);
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
    virtual RuleStatus process(SCAGCommand& command, Session& session);


    Rule(): useCounter(1), transportType(SMPP) {};
    virtual ~Rule();
};

}}

#endif
