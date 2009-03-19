#ifndef _SCAG_PVSS_OLD_SYNCCONFIG_H_
#define _SCAG_PVSS_OLD_SYNCCONFIG_H_

namespace scag2 {
namespace pvss {

class SyncConfig {
public:
  static const int MAX_CLIENTS_COUNT = 1;
  static const uint16_t IO_TASKS_COUNT = 1;
  static const uint16_t IO_TIMEOUT = 300;
  static const uint32_t CONNECT_TIMEOUT = 1000;
  static const int PERF_COUNTER_PERIOD = 10;

public:
  SyncConfig():
  port_(0),
  maxClientsCount_(MAX_CLIENTS_COUNT),
  ioTasksCount_(IO_TASKS_COUNT),
  ioTimeout_(IO_TIMEOUT),
  connectTimeout_(CONNECT_TIMEOUT),
  perfCounterOn_(false),
  perfCounterPeriod_(PERF_COUNTER_PERIOD) 
  {}

  void setPort(int port) {
    port_ = port;
  }
  int getPort() const {
    return port_;
  }
  void setMaxClientsCount(int clientsCount) {
    maxClientsCount_ = clientsCount;
  }
  int getMaxClientsCount() const {
    return maxClientsCount_;
  }
  void setIoTasksCount(int ioTasksCount) {
    ioTasksCount_ = ioTasksCount;
  }
  int getIoTasksCount() const {
    return ioTasksCount_;
  }
  void setIoTimeout(int ioTimeout) {
    ioTimeout_ = ioTimeout;
  }
  int getIoTimeout() const {
    return ioTimeout_;
  }
  void setConnectTimeout(int connectTimeout) {
    connectTimeout_ = connectTimeout;
  }
  int getConnectTimeout() const {
    return connectTimeout_;
  }
  void setPerfCounterOn(bool perfCounterOn) {
    perfCounterOn_ = perfCounterOn;
  }
  bool getPerfCounterOn() const {
    return perfCounterOn_;
  }
  void setPerfCounterPeriod(int perfCounterPeriod) {
    perfCounterPeriod_ = perfCounterPeriod; 
  }
  int getPerfCounterPeriod() const {
    return perfCounterPeriod_;
  }

private:
  int port_;
  int maxClientsCount_;
  uint16_t ioTasksCount_;
  uint16_t ioTimeout_;
  uint32_t connectTimeout_;
  bool perfCounterOn_;
  int perfCounterPeriod_;
};


}
}

#endif

