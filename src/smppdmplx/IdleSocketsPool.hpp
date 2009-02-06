#ifndef __SMPPDMPLX_IDLESOCKETSPOOL_HPP__
# define __SMPPDMPLX_IDLESOCKETSPOOL_HPP__

# include <sys/types.h>
# include <list>
# include <memory>

# include <logger/Logger.h>
# include <util/Singleton.hpp>
# include <smppdmplx/core_ax/network/Socket.hpp>

namespace smpp_dmplx {

/*
** Ответственность класса - предоставить интерфейс для управления таймаутом
** ожидания операций ввода/вывода на сокете.
*/
class IdleSocketsPool : public smsc::util::Singleton<IdleSocketsPool> {
public:
  IdleSocketsPool();

  typedef std::pair<smsc::core_ax::network::Socket, time_t> Socket_Timeout_pair_t;
  typedef std::list<Socket_Timeout_pair_t> IdleSocketList_t;

  /*
  ** Возвращает true - если есть сокеты, для которых истек таймаут ожидания
  ** активности, иначе возвращает false.
  */
  bool getTimedOutSocketsList(IdleSocketList_t* timedOutSocketlist);

  /*
  ** Помещает сокет в пул сокетов, ожидающих выполнения операции ввода/вывода.
  */
  void insertWaitingSocket(const smsc::core_ax::network::Socket& socket, time_t socketTimeoutValue);

  /*
  ** Удаляет сокет, на котором выполнялась операция ввода/вывода, из пула
  */
  void removeActiveSocket(const smsc::core_ax::network::Socket& socket);

private:
  smsc::logger::Logger* _log;
  IdleSocketList_t _idle_socket_list;
};

}
#endif
