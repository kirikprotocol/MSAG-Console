#ifndef __SMPPDMPLX_IDLESOCKETSPOOL_HPP__
# define __SMPPDMPLX_IDLESOCKETSPOOL_HPP__ 1

# include <sys/types.h>
# include <core_ax/network/Socket.hpp>
# include <util/Singleton.hpp>

# include <list>
# include <memory>

namespace smpp_dmplx {

/*
** Ответственность класса - предоставить интерфейс для управления таймаутом
** ожидания операций ввода/вывода на сокете.
*/
class IdleSocketsPool : public smsc::util::Singleton<IdleSocketsPool> {
public:
  typedef std::pair<smsc::core_ax::network::Socket, time_t> Socket_Timeout_pair_t;
  typedef std::list<Socket_Timeout_pair_t> IdleSocketList_t;

  /*
  ** Возвращает true - если есть сокеты, для которых истек таймаут ожидания
  ** активности, иначе возвращает false.
  */
  bool getTimedOutSocketsList(IdleSocketList_t& timedOutSocketlist);

  /*
  ** Помещает сокет в пул сокетов, ожидающих выполнения операции ввода/вывода.
  */
  void insertWaitingSocket(const smsc::core_ax::network::Socket& socket, time_t socketTimeoutValue);

  /*
  ** Удаляет сокет, на котором выполнялась операция ввода/вывода, из пула
  */
  void removeActiveSocket(const smsc::core_ax::network::Socket& socket);

private:
  IdleSocketList_t _idle_socket_list;
};

}
#endif
