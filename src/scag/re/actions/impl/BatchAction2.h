#ifndef _BATCH_PERS_ACTION_COMMIT_
#define _BATCH_PERS_ACTION_COMMIT_

#include <string>
#include "PersAction2.h"

namespace scag2 { namespace re { namespace actions {  

class BatchAction : public PersActionBase, public PersActionResultRetriever
{
public:
    BatchAction() : PersActionBase(), transactMode_(false) {}
    virtual ~BatchAction();
    virtual const char* opname() const { return "profile:batch"; }
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool RunBeforePostpone( ActionContext& context );

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

    virtual PersActionResultRetriever& resultHandler() { return *this; }
    virtual pvss::ProfileCommand* makeCommand( ActionContext& ctx );
    virtual void handleResponse( ActionContext& ctx, const pvss::CommandResponse& resp );
    virtual void handleError( ActionContext& ctx, const pvss::PvssException& e );
    virtual const std::string& statusName() const { return batchStatus_; }
    virtual const std::string& msgName() const { return batchMsg_; }
    virtual const char* typeToString() const { return "batch"; }

private:
    // NOTE: it rethrow exception e
    void handleException( ActionContext& ctx, const pvss::PvssException& e,
                          std::vector< PersActionCommand* >::const_iterator i );

protected:
    std::vector< PersActionCommand* > actions_; // owned
    PropertyObject pobj_;
    bool transactMode_;
    std::string batchStatus_;
    std::string batchMsg_;
};

}
}
}

#endif
