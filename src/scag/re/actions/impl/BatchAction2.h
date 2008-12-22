#ifndef _BATCH_PERS_ACTION_COMMIT_
#define _BATCH_PERS_ACTION_COMMIT_

#include <string>
#include "PersAction2.h"

namespace scag2 { namespace re { namespace actions {  

class BatchAction : public PersActionBase, public PersActionResultRetriever
{
public:
    BatchAction() : PersActionBase(pvss::PC_MTBATCH), transactMode(false) {}
    virtual ~BatchAction();
    virtual bool RunBeforePostpone(ActionContext& context);
    // virtual void ContinueRunning(ActionContext& context);
    virtual void init(const SectionParams& params,PropertyObject propertyObject);

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

    virtual const std::string& statusName() const { return batchStatus; }
    virtual const std::string& msgName() const { return batchMsg; }
    virtual pvss::PersCmd cmdType() const { return pvss::PC_MTBATCH; }
    virtual pvss::PersCommand* makeCommand( ActionContext& ctx );
    virtual PersActionResultRetriever& results() { return *this; }
    virtual void storeResults( const pvss::PersCommand& command, ActionContext& context );

protected:
    std::vector<PersActionCommand *> actions; // owned
    PropertyObject pobj;
    bool transactMode;
    std::string batchStatus;
    std::string batchMsg;
};

}
}
}

#endif
