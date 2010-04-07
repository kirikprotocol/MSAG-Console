#include <utility>

#include "ReassemblyProcessor.hpp"
#include "eyeline/ss7na/common/Exception.hpp"
#include "eyeline/ss7na/common/types.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

void
ReassemblyProcessor::destroyReassembling(const messages::TLV_SourceAddress& calling_addr,
                                         uint32_t segment_local_ref)
{
  smsc_log_debug(_logger, "ReassemblyProcessor::destroyReassembling::: calling_addr='%s', segment_local_ref=0x%06X",
                 calling_addr.toString().c_str(), segment_local_ref);
  smsc::core::synchronization::MutexGuard synchronize(_registryLock);
  ReassemblyProcessKey searchKey(calling_addr, segment_local_ref);
  reassembleRegistry::iterator iter = _activeReassembleProcess.find(searchKey);
  if ( iter == _activeReassembleProcess.end() )
    return;

  if ( !iter->second.destroyed )
    for (unsigned idx = 0; idx < ReassembleInfo::MAX_SEGMENTS_COUNT; ++idx) {
      smsc_log_debug(_logger, "ReassemblyProcessor::destroyReassembling::: destroy message part with idx=%u", idx);
      delete iter->second.msgPart[idx]; iter->second.msgPart[idx] = NULL;
    }

  smsc_log_debug(_logger, "ReassemblyProcessor::destroyReassembling::: reassembling info for key='callingAddr=%s,segmentLocalref=0x%06X' has been destroyed",
                 searchKey.callingAddr.toString().c_str(), searchKey.segmentLocalRef);

  _activeReassembleProcess.erase(iter);
}

}}}}
