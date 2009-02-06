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
** ��������������� ������ - ������������ ��������� ��� ���������� ���������
** �������� �������� �����/������ �� ������.
*/
class IdleSocketsPool : public smsc::util::Singleton<IdleSocketsPool> {
public:
  IdleSocketsPool();

  typedef std::pair<smsc::core_ax::network::Socket, time_t> Socket_Timeout_pair_t;
  typedef std::list<Socket_Timeout_pair_t> IdleSocketList_t;

  /*
  ** ���������� true - ���� ���� ������, ��� ������� ����� ������� ��������
  ** ����������, ����� ���������� false.
  */
  bool getTimedOutSocketsList(IdleSocketList_t* timedOutSocketlist);

  /*
  ** �������� ����� � ��� �������, ��������� ���������� �������� �����/������.
  */
  void insertWaitingSocket(const smsc::core_ax::network::Socket& socket, time_t socketTimeoutValue);

  /*
  ** ������� �����, �� ������� ����������� �������� �����/������, �� ����
  */
  void removeActiveSocket(const smsc::core_ax::network::Socket& socket);

private:
  smsc::logger::Logger* _log;
  IdleSocketList_t _idle_socket_list;
};

}
#endif
