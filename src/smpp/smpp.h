/*
  $Id$
*/

#include "util/debug.h"
#include "sms/sms.h"
#include <netinet/in.h>
#include <inttypes.h>
#include <algorithm>
#include <memory>

#define SMPP_USE_BUFFER
#define SMPP_SHARE_BUFFER_MEMORY

namespace smsc {
namespace smpp {

using smsc::sms::SMS;
using smsc::sms::Address;
using std::auto_ptr;


#include "smpp_structures.h"
#include "smpp_stream.h"

#include "smpp_strings.h"
#include "smpp_optional.h"
#include "smpp_mandatory.h"
#include "smpp_time.h"
#include "smpp_sms.h"

#undef __check_stream_invariant__

}; // smpp namespace
}; // smsc namespace
