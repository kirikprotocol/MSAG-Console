#ifndef _SCAG_PVSS_BASE_BATCHCOMMAND_H
#define _SCAG_PVSS_BASE_BATCHCOMMAND_H

#include <vector>
#include "ProfileCommand.h"
#include "ProfileCommandVisitor.h"
#include "BatchRequestComponent.h"

namespace scag2 {
namespace pvss {

class BatchCommand : public ProfileCommand
{
public:
    BatchCommand() : ProfileCommand(), transactional_(false) { initLog(); }
    // BatchCommand( uint32_t seqNum ) : ProfileCommand(seqNum) {}

    virtual ~BatchCommand() {
        logDtor();
        clear();
    }
    
    virtual bool isValid( PvssException* exc = 0 ) const;
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
        // req->setSeqNum(int(batchContent_.size()));
    }
    void addComponents( std::vector< BatchRequestComponent* >& comps ) {
        batchContent_.insert( batchContent_.end(), comps.begin(), comps.end() );
    }

    virtual bool visit( ProfileCommandVisitor& visitor ) // throw ( PvapException )
    {
        return visitor.visitBatchCommand(*this);
    }

    const std::vector< BatchRequestComponent* >&  getBatchContent() const { return batchContent_; }

    virtual BatchCommand* clone() const { return new BatchCommand(*this); }

    virtual const char* typeToString() const { return "batch"; }

private:
    BatchCommand( const BatchCommand& cmd );

private:
    std::vector< BatchRequestComponent* >  batchContent_;
    bool transactional_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_BATCHCOMMAND_H */
