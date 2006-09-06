#ifndef  __SMPPDMPLX_PENDINGOUTDATAQUEUE_HPP__
# define __SMPPDMPLX_PENDINGOUTDATAQUEUE_HPP__ 1

# include <core_ax/network/Socket.hpp>
# include <deque>
# include <map>
# include "BufferedOutputStream.hpp"
# include <string.h>

namespace smpp_dmplx {

/*
**  ласс предоставл€ет интерфейс дл€ отложенной передачи данных адресату.
** ƒанные планируютс€ дл€ передачи через вызовов scheduleDataForSending.
** Ќепосредственно передача данных выполн€етс€ в вызове sendScheduledData - 
** передаютс€ данные, запланированные к передаче дл€ сокета, указанного 
** в аргументе. «а один вызов sendScheduledData отплавл€етс€ ровно столько
** данных, сколько может быть отправлено без блокировани€ сокета. “.о. дл€
** передачи всего объема данных, запланированного дл€ данного сокета,
** может потребоватьс€ несколько вызовов метода sendScheduledData
*/
class PendingOutDataQueue {
public:
  static void scheduleDataForSending(BufferedOutputStream& outBuf, smsc::core_ax::network::Socket& socket);
  static void sendScheduledData(smsc::core_ax::network::Socket& socket);
private:
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

  typedef std::deque<data_t> scheduled_data_t ;
  typedef std::map<smsc::core_ax::network::Socket, scheduled_data_t> sched_table_t;

  static sched_table_t sched_table;
};

}

#endif
