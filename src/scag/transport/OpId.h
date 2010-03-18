#ifndef SCAG_TRANSPORT_OPID_H
#define SCAG_TRANSPORT_OPID_H

#include "util/int.h"

namespace scag2 {
namespace transport {

typedef uint32_t opid_type;
inline opid_type invalidOpId() { return opid_type(0); }

}
}

namespace scag {
namespace transport {
using scag2::transport::opid_type;
using scag2::transport::invalidOpId;
}
}

#endif /* !SCAG_TRANSPORT_OPID_H */
