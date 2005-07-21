#ifndef __SCAG_RULE_H__
#define __SCAG_RULE_H__

#include "EventHandler.h"
#include "scag/re/actions/Action.h"
#include "scag/re/actions/ActionIf.h"
#include "scag/re/actions/ActionSet.h"
#include "scag/re/actions/ActionReturn.h"

//#include "scag/re/actions/ActionChoose.h"
#include "SAX2Print.hpp"
#include "scag/transport/SCAGCommand.h"


namespace scag { namespace re 
{

using smsc::core::buffers::IntHash;
using namespace scag::re::actions;
using scag::transport::SCAGCommand;


class EventHandler;
 
class ActionFactory
{
public: 
    Action * CreateAction(const std::string& name) const;
    void FillTagHash(smsc::core::buffers::Hash<int>& TagHash) const;
};

class Rule : public IParserHandler
{
    Rule(const Rule &);
    IntHash <EventHandler *> Handlers;
    Mutex ruleLock;
    int useCounter;

protected:
    std::string billing_id;

//////////////IParserHandler Interfase///////////////////////
    virtual void SetChildObject(IParserHandler * child);
    virtual void StartXMLSubSection(const std::string& name,const SectionParams& params) {};
    virtual void FinishXMLSubSection(const std::string& name) {};
//////////////IParserHandler Interfase///////////////////////
public:
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

    /**
     * Creates & configure rule attributes and
     * handlers from sub-section (via HandlersFactory ?)
     * @param   config      config sub-section for rule
     */
//    virtual void init(ConfigView* config) = 0;


    /** 
     * Processes command via one of rules's handler.
     * Searches handler by command id/type and
     * returns RuleStatus after handler execution.
     * @param   command     command to process
     * @return  status      rule's handler execution status
     */
    virtual RuleStatus process(SCAGCommand command);


    Rule(const SectionParams& params): useCounter(1) {};
    virtual ~Rule();
};

}}

#endif
