#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_REASSSEMBLYPROCESSOR_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_REASSSEMBLYPROCESSOR_HPP__

# include <map>
# include <list>
# include <string.h>
# include <sys/types.h>
# include "logger/Logger.h"
# include "util/Exception.hpp"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/TimeoutMonitor.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/UDT.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/XUDT.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class ReassemblyProcessor : public utilx::Singleton<ReassemblyProcessor> {
public:
  void initialize(unsigned reassembly_timer) {
    _reassemblyTimer = reassembly_timer;
    _timerMonitor->initialize(reassembly_timer + 1);
  }
  void reinitialize(unsigned reassembly_timer) {
    smsc_log_debug(_logger, "ReassemblyProcessor::reinitialize::: reassembly_timer=%u",
                   reassembly_timer);
    if ( _reassemblyTimer == reassembly_timer )
      return;

    smsc::core::synchronization::MutexGuard synchronize(_timerMonitorLock);
    for ( cancelled_tmonitors::iterator iter = _cancelledTimerMonitors.begin(), end_iter = _cancelledTimerMonitors.end();
         iter != end_iter; ) {
      if ( (*iter)->stopped() ) {
        delete *iter;
        _cancelledTimerMonitors.erase(iter++);
      } else
        iter++;
    }
    _timerMonitor->shutdown();
    _cancelledTimerMonitors.push_back(_timerMonitor);
    _timerMonitor = new utilx::TimeoutMonitor(++_timeoutIdSeed);
    _timerMonitor->initialize(reassembly_timer + 1);
  }

  // return true if full message has been reassembled.
  // In this case buff_for_msg_data will contain full assembled message data.
  template <class MSG>
  bool reassemble(const MSG& next_msg_part,
                  uint8_t* buff_for_msg_data, uint16_t* real_data_sz, size_t max_buf_sz);
  template <class MSG>
  void cancelReassembling(const MSG& next_msg_part);
  void destroyReassembling(const eyeline::sccp::SCCPAddress& calling_addr,
                           uint32_t segment_local_ref);
  enum { NeedNextMessagePart = false, FullMessageReassembled = true };
private:
  ReassemblyProcessor()
  : _reassemblyTimer(0), _logger(smsc::logger::Logger::getInstance("sccp")),
    _timerMonitor(new utilx::TimeoutMonitor()), _timeoutIdSeed(0)
  {}

  friend class utilx::Singleton<ReassemblyProcessor>;

  struct DataPortion {
    DataPortion(const uint8_t* data_portion, uint16_t data_portion_sz)
    : dataPortionSz(data_portion_sz)
    {
      if ( data_portion_sz > sizeof(dataPortion) )
        throw smsc::util::Exception("ReassemblyProcessor::DataPortion::: too long data_portion, max size = %u, got portion with size = %u",
                                     sizeof(dataPortion), data_portion_sz);
      memcpy(dataPortion, data_portion, data_portion_sz);
    }
    uint8_t dataPortion[272];
    uint16_t dataPortionSz;
  };
  struct ReassembleInfo {
    enum { MAX_SEGMENTS_COUNT = 16 };
    ReassembleInfo()
    : expectedSegCount(0), currentSegCount(0), destroyed(false), timerId(0) {
      memset(msgPart, 0, sizeof(DataPortion*) * MAX_SEGMENTS_COUNT);
    }
    unsigned expectedSegCount, currentSegCount;
    bool destroyed;
    utilx::TimeoutMonitor::timeout_id_t timerId;
    DataPortion* msgPart[MAX_SEGMENTS_COUNT];
  };
  struct ReassemblyProcessKey {
    ReassemblyProcessKey(const eyeline::sccp::SCCPAddress& calling_addr,
                         uint32_t  segment_local_ref)
    : callingAddr(calling_addr), segmentLocalRef(segment_local_ref)
    {}

    bool operator< (const ReassemblyProcessKey& rhs) const {
      if ( callingAddr.toString() != rhs.callingAddr.toString() )
        return callingAddr.toString() < rhs.callingAddr.toString();
      if ( segmentLocalRef != rhs.segmentLocalRef )
        return segmentLocalRef < rhs.segmentLocalRef;
      return false;
    }
    eyeline::sccp::SCCPAddress callingAddr;
    uint32_t segmentLocalRef;
  };
  typedef std::map<ReassemblyProcessKey, ReassembleInfo> reassembleRegistry;
  reassembleRegistry _activeReassembleProcess;
  smsc::core::synchronization::Mutex _registryLock;
  unsigned _reassemblyTimer;
  smsc::logger::Logger* _logger;
  utilx::TimeoutMonitor* _timerMonitor;
  unsigned _timeoutIdSeed;
  typedef std::list<utilx::TimeoutMonitor*> cancelled_tmonitors;
  cancelled_tmonitors _cancelledTimerMonitors;
  smsc::core::synchronization::Mutex _timerMonitorLock;
};

}}}}

# include "eyeline/ss7na/m3ua_gw/sccp/ReassemblyProcessor_impl.hpp"

#endif
