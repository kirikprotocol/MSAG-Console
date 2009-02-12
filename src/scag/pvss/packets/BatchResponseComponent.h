#ifndef _SCAG_PVSS_BASE_BATCHRESPONSECOMPONENT_H
#define _SCAG_PVSS_BASE_BATCHRESPONSECOMPONENT_H

#include "Response.h"

namespace scag2 {
namespace pvss {

class BatchResponseComponent : public Response
{
public:
    BatchResponseComponent() : Response() {}
    BatchResponseComponent( uint32_t seqNum ) : Response(seqNum) {}
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_BATCHRESPONSECOMPONENT_H */
