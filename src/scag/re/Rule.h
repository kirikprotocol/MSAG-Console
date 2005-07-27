#ifndef __SCAG_RULE_H__
#define __SCAG_RULE_H__

#include "EventHandler.h"
#include "scag/transport/SCAGCommand.h"
#include "RuleStatus.h"


namespace scag { namespace re 
{

using smsc::core::buffers::IntHash;
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
    int HandlerTypeToInt(const std::string& str);

protected:
    std::string billing_id;
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

    virtual void init(const SectionParams& params);


    /** 
     * Processes command via one of rules's handler.
     * Searches handler by command id/type and
     * returns RuleStatus after handler execution.
     * @param   command     command to process
     * @return  status      rule's handler execution status
     */
    virtual RuleStatus process(SCAGCommand& command);


    Rule(): useCounter(1),transportType(SMPP) {};
    virtual ~Rule();
};

}}

#endif
