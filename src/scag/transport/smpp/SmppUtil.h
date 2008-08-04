#ifndef __SCAG_TRANSPORT_SMPP_SMPPUTIL_H__
#define __SCAG_TRANSPORT_SMPP_SMPPUTIL_H__

#include <vector>
#include "sms/sms.h"
#include "core/buffers/TmpBuf.hpp"

namespace scag{
namespace transport{
namespace smpp{

void stripUnknownSmppOptionals(smsc::sms::SMS& sms,const std::vector<int>& allowedTags);

}//smpp
}//transport
}//scag

#endif
