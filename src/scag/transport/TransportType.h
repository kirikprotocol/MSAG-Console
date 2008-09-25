#ifndef _SCAG_TRANSPORT_TRANSPORTTYPE_H
#define _SCAG_TRANSPORT_TRANSPORTTYPE_H

namespace scag {
namespace transport {

    typedef enum TransportType
    {
        SMPP = 1,
        HTTP = 2,
        MMS = 3
    } TransportType;
    
} // namespace transport
} // namespace scag

namespace scag2 {
namespace transport {
using scag::transport::TransportType;
using scag::transport::SMPP;
using scag::transport::HTTP;
using scag::transport::MMS;
}
}


#endif /* !_SCAG_TRANSPORT_TRANSPORTTYPE_H */
