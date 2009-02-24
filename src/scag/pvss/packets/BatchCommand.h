#ifndef _SCAG_PVSS_BASE_BATCHCOMMAND_H
#define _SCAG_PVSS_BASE_BATCHCOMMAND_H

#include <vector>
#include "AbstractCommand.h"
#include "ProfileCommandVisitor.h"

namespace scag2 {
namespace pvss {

class BatchRequestComponent;

class BatchCommand : public AbstractCommand
{
public:
    BatchCommand() : AbstractCommand() {}
    BatchCommand( uint32_t seqNum ) : AbstractCommand(seqNum) {}
    virtual ~BatchCommand() {
        clear();
    }
    
    virtual bool isValid() const;
    virtual std::string toString() const;

    bool isTransactional() {
        return transactional_;
    }

    void setTransactional( bool transactional ) {
        transactional_ = transactional;
    }

    void clear();

    void addComponent( BatchRequestComponent* req ) {
        batchContent_.push_back( req );
    }

    virtual bool visit( ProfileCommandVisitor& visitor ) throw ( PvapException )
    {
        return visitor.visitBatchCommand(*this);
    }

    const std::vector< BatchRequestComponent* >&  getBatchContent() const { return batchContent_; }

    virtual BatchCommand* clone() const { return new BatchCommand(*this); }

protected:
    virtual const char* typeToString() const { return "batch"; }
    virtual ResponseTypeMatch& getResponseTypeMatch() const;

private:
    BatchCommand( const BatchCommand& cmd );

private:
    std::vector< BatchRequestComponent* >  batchContent_;
    bool transactional_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_BATCHCOMMAND_H */
