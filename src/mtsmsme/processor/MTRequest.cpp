static char const ident[] = "$Id$";
#include "MTRequest.hpp"
#include "MTFTSM.hpp"
namespace smsc{namespace mtsmsme{namespace processor{
void MTR::setSendResult(int result)
{
  tsm->sendResponse(result,invokeId);
}
}}}
