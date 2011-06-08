#ifndef _SCAG_PVSS_BASE_BATCHRESPONSE_H
#define _SCAG_PVSS_BASE_BATCHRESPONSE_H

#include <vector>
#include "CommandResponse.h"
#include "BatchResponseComponent.h"

namespace scag2 {
namespace pvss {

class BatchResponse : public CommandResponse
{
public:
    BatchResponse() : CommandResponse() { initLog(); }
    BatchResponse( uint8_t status ) : CommandResponse(status) { initLog(); }
    virtual ~BatchResponse() { logDtor(); clear(); }

    virtual bool isValid( PvssException* exc = 0 ) const {
        CHECKMAGTC;
        return CommandResponse::isValid(exc);
    }

    virtual bool visit( ProfileResponseVisitor& visitor ) /* throw (PvapException) */  {
        CHECKMAGTC;
        return visitor.visitBatchResponse(*this);
    }

    virtual BatchResponse* clone() const {
        CHECKMAGTC;
        return new BatchResponse(*this);
    }

    void addComponent( BatchResponseComponent* resp ) {
        CHECKMAGTC;
        if ( batchContent_.empty() || getStatus() == OK ) {
            setStatus( resp->getStatus() );
        }
        batchContent_.push_back( resp );
    }

    const std::vector< BatchResponseComponent* >& getBatchContent() const {
        CHECKMAGTC;
        return batchContent_;
    }

    std::string toString() const {
        CHECKMAGTC;
        std::string rv(CommandResponse::toString());
        rv.reserve(200);
        rv += " [";
        bool comma = false;
        for ( std::vector< BatchResponseComponent* >::const_iterator i = batchContent_.begin();
              i != batchContent_.end();
              ++i ) {
            if ( comma ) rv += ", ";
            else comma = true;
            rv += (*i)->toString();
        }
        rv += "]";
        return rv;
    }

    void clear() {
        CHECKMAGTC;
        CommandResponse::clear();
        for ( std::vector< BatchResponseComponent* >::iterator i = batchContent_.begin();
              i != batchContent_.end();
              ++i ) {
            delete *i;
        }
        batchContent_.clear();
    }

protected:
    virtual const char* typeToString() const { return "batch_resp"; }

private:
    BatchResponse( const BatchResponse& other ) : CommandResponse(other) {
        for ( std::vector< BatchResponseComponent* >::const_iterator i = other.batchContent_.begin();
              i != other.batchContent_.end();
              ++i ) {
            batchContent_.push_back( (*i)->clone() );
        }
    }
    BatchResponse& operator = ( const BatchResponse& );

private:
    std::vector< BatchResponseComponent* > batchContent_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_SETRESPONSE_H */
