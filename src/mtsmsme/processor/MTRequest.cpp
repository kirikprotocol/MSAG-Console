#include "MTRequest.hpp"
#include "TSM.hpp"
namespace smsc{namespace mtsmsme{namespace processor{
void MTR::setSendResult(int result)
{
  tsm->sendResponse(result,invokeId);
}
}}}
