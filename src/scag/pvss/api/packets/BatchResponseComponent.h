#ifndef _SCAG_PVSS_BASE_BATCHRESPONSECOMPONENT_H
#define _SCAG_PVSS_BASE_BATCHRESPONSECOMPONENT_H

#include "CommandResponse.h"

namespace scag2 {
namespace pvss {

class BatchResponseComponent : public CommandResponse
{
public:
    BatchResponseComponent() : CommandResponse() {}
    BatchResponseComponent( uint8_t status ) : CommandResponse( status ) {}
    virtual BatchResponseComponent* clone() const = 0;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_BATCHRESPONSECOMPONENT_H */
