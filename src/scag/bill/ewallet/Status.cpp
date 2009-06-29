#include "Status.h"

namespace scag2 {
namespace bill {
namespace ewallet {

const char* Status::statusToString( uint8_t status )
{
#define STATUSTOSTRING(x) case (x) : return #x
    switch (status) {
        STATUSTOSTRING(OK            );
        STATUSTOSTRING(BAD_REQUEST   );
        STATUSTOSTRING(BAD_RESPONSE  );
        STATUSTOSTRING(TIMEOUT       );
        STATUSTOSTRING(IO_ERROR      );
        STATUSTOSTRING(NOT_SUPPORTED );
        STATUSTOSTRING(NOT_CONNECTED );
        STATUSTOSTRING(CLIENT_BUSY   );
        STATUSTOSTRING(CONFIG_INVALID);
        STATUSTOSTRING(UNKNOWN       );
    default: return "???";
    }
#undef STATUSTOSTRING
}

} // namespace ewallet
} // namespace bill
} // namespace scag2
