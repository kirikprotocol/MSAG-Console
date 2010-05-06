namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class ReassemblyTimerExpirationHandler : public utilx::TimeoutHandler {
public:
  ReassemblyTimerExpirationHandler(const eyeline::sccp::SCCPAddress& calling_addr,
                                   uint32_t segment_local_ref)
  : _callingAddr(calling_addr), _segmentLocalRef(segment_local_ref),
    _logger(smsc::logger::Logger::getInstance("sccp"))
  {}

  virtual void handle() {
    ReassemblyProcessor::getInstance().destroyReassembling(_callingAddr, _segmentLocalRef);
  }
private:
  eyeline::sccp::SCCPAddress _callingAddr;
  uint32_t _segmentLocalRef;
  smsc::logger::Logger* _logger;
};

template <class MSG>
bool
ReassemblyProcessor::reassemble(const MSG& next_msg_part,
                                uint8_t* buff_for_msg_data, uint16_t* real_data_sz,
                                size_t max_buf_sz)
{
  const messages::Segmentation& segmentation = next_msg_part.getSegmentation();
  smsc_log_debug(_logger, "ReassemblyProcessor::reassemble::: segmentation info for current msg='%s'",
                 segmentation.toString().c_str());

  smsc::core::synchronization::MutexGuard synchronize(_registryLock);
  ReassemblyProcessKey searchKey(next_msg_part.getCallingAddress(),
                                 segmentation.getLocalReference());
  reassembleRegistry::iterator iter = _activeReassembleProcess.find(searchKey);
  if ( iter == _activeReassembleProcess.end() ) {
    std::pair<reassembleRegistry::iterator, bool> insRes =
        _activeReassembleProcess.insert(std::make_pair(searchKey, ReassembleInfo()));
    iter = insRes.first;
    smsc::core::synchronization::MutexGuard timerMomitorSynch(_timerMonitorLock);
    iter->second.timerId =
        _timerMonitor->schedule(_reassemblyTimer,
                                new ReassemblyTimerExpirationHandler(next_msg_part.getCallingAddress(),
                                                                     segmentation.getLocalReference()));
  } else if ( iter->second.destroyed )
    throw common::SCCPException(common::DESTINATION_CANNOT_PERFORM_REASSEMBLY,
                                "ReassemblyProcessor::reassemble::: active reassemble process has been destroyed");

  utilx::variable_data_t msgPart = next_msg_part.getData();
  iter->second.msgPart[segmentation.getRemainingSegments()] = new DataPortion(msgPart.data, msgPart.dataLen);
  if ( segmentation.isFirstSegment() )
    iter->second.expectedSegCount = segmentation.getRemainingSegments() + 1;

  ++iter->second.currentSegCount;

  if ( iter->second.currentSegCount == iter->second.expectedSegCount ) {
    int idx = iter->second.expectedSegCount - 1;
    unsigned offset = 0;
    do {
      if ( !iter->second.msgPart[idx] )
        throw common::SCCPException(common::DESTINATION_CANNOT_PERFORM_REASSEMBLY,
                                    "ReassemblyProcessor::reassemble::: fatal error - message part with idx=%u has not been received",
                                    idx);
      if ( offset + iter->second.msgPart[idx]->dataPortionSz > max_buf_sz )
        throw common::SCCPException(common::DESTINATION_CANNOT_PERFORM_REASSEMBLY,
                                    "ReassemblyProcessor::reassemble::: out buffer too small - max_buf_sz=%u, need at least %u bytes",
                                    max_buf_sz, offset + iter->second.msgPart[idx]->dataPortionSz);

      memcpy(buff_for_msg_data + offset, iter->second.msgPart[idx]->dataPortion,
             iter->second.msgPart[idx]->dataPortionSz);
      offset += iter->second.msgPart[idx]->dataPortionSz;
      delete iter->second.msgPart[idx];
    } while (--idx >= 0);
    *real_data_sz = offset;
    smsc::core::synchronization::MutexGuard timerMomitorSynch(_timerMonitorLock);
    if ( !_timerMonitor->cancel(iter->second.timerId) ) {
      for (cancelled_tmonitors::iterator c_iter = _cancelledTimerMonitors.begin(), c_end_iter = _cancelledTimerMonitors.end();
           c_iter != c_end_iter; ++c_iter) {
        if ( (*c_iter)->cancel(iter->second.timerId) )
          break;
      }
    }
    _activeReassembleProcess.erase(iter);
    return FullMessageReassembled;
  }
  return NeedNextMessagePart;
}

template <class MSG>
void
ReassemblyProcessor::cancelReassembling(const MSG& next_msg_part)
{
  const messages::Segmentation& segmentation = next_msg_part.getSegmentation();

  smsc::core::synchronization::MutexGuard synchronize(_registryLock);
  ReassemblyProcessKey searchKey(next_msg_part.getCallingAddress(),
                                 segmentation.getLocalReference());
  reassembleRegistry::iterator iter = _activeReassembleProcess.find(searchKey);
  if ( iter == _activeReassembleProcess.end() || iter->second.destroyed )
    return;

  for (unsigned idx = 0; idx < ReassembleInfo::MAX_SEGMENTS_COUNT; ++idx) {
    smsc_log_debug(_logger, "ReassemblyProcessor::cancelReassembling::: destroy message part with idx=%u", idx);
    delete iter->second.msgPart[idx]; iter->second.msgPart[idx] = NULL;
  }

  smsc_log_debug(_logger, "ReassemblyProcessor::cancelReassembling::: reassembling info for key='callingAddr=%s,segmentLocalref=%u' has been destroyed",
                 searchKey.callingAddr.toString().c_str(), searchKey.segmentLocalRef);
  iter->second.destroyed = true;
}

}}}}
