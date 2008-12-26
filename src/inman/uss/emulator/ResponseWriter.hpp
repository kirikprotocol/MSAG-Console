#ifndef __SMSC_INMAN_USS_EMULATOR_RESPONSEWRITER_HPP__
# define __SMSC_INMAN_USS_EMULATOR_RESPONSEWRITER_HPP__

# include <map>
# include <list>

# include <sys/time.h>
# include <logger/Logger.h>
# include <core/threads/Thread.hpp>
# include <core/synchronization/EventMonitor.hpp>
# include <core/synchronization/Mutex.hpp>

# include <inman/uss/ussmessages.hpp>
# include <inman/interaction/connect.hpp>
# include <inman/interaction/serializer.hpp>
# include <inman/uss/emulator/RequestProcessingRules.hpp>
# include <util/Singleton.hpp>

namespace smsc {
namespace inman {
namespace uss {

class ResponseWriter : public core::threads::Thread {
public:
  explicit ResponseWriter(unsigned responseDelayInMsecs)
    : _responseDelayInMsecs(responseDelayInMsecs), _logger(logger::Logger::getInstance("rspWrtr")) {}
  virtual int Execute();

  void scheduleResponse(interaction::SPckUSSResult* result, interaction::Connect* conn);
  void cancelScheduledResponse(unsigned connId);
private:
  struct output_item {
    output_item(const struct timeval& aTimeToActivate,
                interaction::SPckUSSResult* aResult,
                unsigned aConnId)
      : timeToAcivate(aTimeToActivate), result(aResult), connId(aConnId) {}

    std::string toString() const {
      char strBuf[1024];
      snprintf(strBuf, sizeof(strBuf), "timeToAcivate.tv_sec=%d,timeToAcivate.tv_usec=%d,uss_result=[%s],connId=%d", timeToAcivate.tv_sec, timeToAcivate.tv_usec, result->Cmd().toString().c_str(), connId);

      return strBuf;
    }

    struct timeval timeToAcivate;
    interaction::SPckUSSResult* result;
    unsigned connId;
  };

  unsigned _responseDelayInMsecs;
  logger::Logger* _logger;
  core::synchronization::EventMonitor _eventMonitor;

  typedef std::list<output_item> output_queue_t;
  output_queue_t _output_queue;

  struct connect_info {
    explicit connect_info(interaction::Connect* aConnect)
      : connect(aConnect) {}
    interaction::Connect* connect;
    core::synchronization::Mutex connectLock;
  };
  typedef std::map<unsigned/*connId*/, connect_info*> connect_registry_t;
  connect_registry_t _connectRegistry;
  core::synchronization::Mutex _connRegistryLock;
  output_item getReadyToDeliveryResponse();
};

}}}

#endif
