#include "PersServerResponse.h"

namespace {

const char* RESPONSE_TEXT[] = {
  "unknown",
  "ok",
  "error",
  "property not found",
  "bad request",
  "type inconsistence",
  "command in process",
  "profile locked",
  "not support"
};

}

namespace scag2 {
namespace pvss {
namespace perstypes {

const char* persServerResponse( int resp )
{
    if ( resp < 0 || resp > sizeof(RESPONSE_TEXT)/sizeof(const char*) ) {
        return "???";
    } else {
        return RESPONSE_TEXT[resp];
    }
}

}
}
}
