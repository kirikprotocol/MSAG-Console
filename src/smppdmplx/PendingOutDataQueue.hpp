#ifndef  __SMPPDMPLX_PENDINGOUTDATAQUEUE_HPP__
# define __SMPPDMPLX_PENDINGOUTDATAQUEUE_HPP__

# include <string.h>
# include <deque>
# include <map>

# include <logger/Logger.h>
# include <util/Singleton.hpp>
# include <smppdmplx/BufferedOutputStream.hpp>
# include <smppdmplx/core_ax/network/Socket.hpp>

namespace smpp_dmplx {

/*
** Класс предоставляет интерфейс для отложенной передачи данных адресату.
** Данные планируются для передачи через вызовов scheduleDataForSending.
** Непосредственно передача данных выполняется в вызове sendScheduledData - 
** передаются данные, запланированные к передаче для сокета, указанного 
** в аргументе. За один вызов sendScheduledData отплавляется ровно столько
** данных, сколько может быть отправлено без блокирования сокета. Т.о. для
** передачи всего объема данных, запланированного для данного сокета,
** может потребоваться несколько вызовов метода sendScheduledData
*/
class PendingOutDataQueue : public smsc::util::Singleton<PendingOutDataQueue> {
public:
  PendingOutDataQueue();

  void scheduleDataForSending(BufferedOutputStream& outBuf, smsc::core_ax::network::Socket& socket);
  void sendScheduledData(smsc::core_ax::network::Socket& socket);
  void cancelScheduledData(const smsc::core_ax::network::Socket& socket);
private:
  smsc::logger::Logger* _log;

  struct data_t {
    data_t(void* aBuffer, size_t aFullBufferSize) :
      fullBufferSize(aFullBufferSize), pos(0) {
      buffer = new uint8_t [fullBufferSize];
      memcpy(buffer, aBuffer, fullBufferSize);
    }

    uint8_t* buffer;
    size_t fullBufferSize;
    size_t pos;
  };

  typedef std::deque<data_t> scheduled_data_t;
  typedef std::map<smsc::core_ax::network::Socket, scheduled_data_t> sched_table_t;

  sched_table_t _sched_table;
};

}

#endif
