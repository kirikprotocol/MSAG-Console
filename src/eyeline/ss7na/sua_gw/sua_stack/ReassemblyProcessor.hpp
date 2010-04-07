#ifndef __EYELINE_SS7NA_SUAGW_SCCP_REASSSEMBLYPROCESSOR_HPP__
# define __EYELINE_SS7NA_SUAGW_SCCP_REASSSEMBLYPROCESSOR_HPP__

# include <map>
# include <string.h>
# include <sys/types.h>
# include "logger/Logger.h"
# include "util/Exception.hpp"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/TimeoutMonitor.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/CLDTMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

class ReassemblyProcessor : public utilx::Singleton<ReassemblyProcessor> {
public:
  void initialize(unsigned reassembly_timer) {
    _reassemblyTimer = reassembly_timer;
    _timerMonitor.initialize(reassembly_timer + 1);
  }
  // return true if full message has been reassembled.
  // In this case buff_for_msg_data will contain full assembled message data.
  template <class MSG>
  bool reassemble(const MSG& next_msg_part,
                  uint8_t* buff_for_msg_data, uint16_t* real_data_sz, size_t max_buf_sz);
  template <class MSG>
  void cancelReassembling(const MSG& next_msg_part);
  void destroyReassembling(const messages::TLV_SourceAddress& calling_addr,
                           uint32_t segment_local_ref);
  enum { NeedNextMessagePart = false, FullMessageReassembled = true };
private:
  ReassemblyProcessor()
  : _reassemblyTimer(0), _logger(smsc::logger::Logger::getInstance("sccp"))
  {}

  friend class utilx::Singleton<ReassemblyProcessor>;

  struct DataPortion {
    DataPortion(const uint8_t* data_portion, uint16_t data_portion_sz)
    : dataPortionSz(data_portion_sz)
    {
      dataPortion = new uint8_t[data_portion_sz];
      memcpy(dataPortion, data_portion, data_portion_sz);
    }
    ~DataPortion() {
      delete [] dataPortion;
    }
    uint8_t* dataPortion;
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
    ReassemblyProcessKey(const messages::TLV_SourceAddress& calling_addr,
                         uint32_t  segment_local_ref)
    : callingAddr(calling_addr), segmentLocalRef(segment_local_ref)
    {}

    bool operator< (const ReassemblyProcessKey& rhs) const {
      if ( callingAddr.messages::TLV_Address::toString() != rhs.callingAddr.toString() )
        return callingAddr.toString() < rhs.callingAddr.toString();
      if ( segmentLocalRef != rhs.segmentLocalRef )
        return segmentLocalRef < rhs.segmentLocalRef;
      return false;
    }
    messages::TLV_SourceAddress callingAddr;
    uint32_t segmentLocalRef;
  };
  typedef std::map<ReassemblyProcessKey, ReassembleInfo> reassembleRegistry;
  reassembleRegistry _activeReassembleProcess;
  smsc::core::synchronization::Mutex _registryLock;
  unsigned _reassemblyTimer;
  smsc::logger::Logger* _logger;
  utilx::TimeoutMonitor _timerMonitor;
};

}}}}

# include "eyeline/ss7na/sua_gw/sua_stack/ReassemblyProcessor_impl.hpp"

#endif
