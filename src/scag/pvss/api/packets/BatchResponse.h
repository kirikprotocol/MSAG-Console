#ifndef _SCAG_PVSS_BASE_BATCHRESPONSE_H
#define _SCAG_PVSS_BASE_BATCHRESPONSE_H

#include "Response.h"
#include "BatchResponseComponent.h"

namespace scag2 {
namespace pvss {

class BatchResponse : public Response
{
public:
    BatchResponse() : Response() {}
    BatchResponse( uint32_t seqNum ) : Response(seqNum) {}
    virtual ~BatchResponse() { clear(); }

    virtual bool isValid() const { return Response::isValid() && !batchContent_.empty(); }
    virtual bool visit( ResponseVisitor& visitor ) throw (PvapException) {
        return visitor.visitBatchResponse(*this);
    }

    virtual BatchResponse* clone() const { return new BatchResponse(*this); }

    void addComponent( BatchResponseComponent* resp ) {
        if ( batchContent_.empty() || getStatus() == OK ) {
            setStatus( resp->getStatus() );
        }
        batchContent_.push_back( resp );
    }

    const std::vector< BatchResponseComponent* >& getBatchContent() const { return batchContent_; }

    std::string toString() const {
        std::string rv(Response::toString());
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
    BatchResponse( const BatchResponse& other ) : Response(other) {
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
