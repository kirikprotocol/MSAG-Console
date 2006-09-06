#ifndef __SMPPDMPLX_IDLESOCKETSPOOL_HPP__
# define __SMPPDMPLX_IDLESOCKETSPOOL_HPP__ 1

# include <sys/types.h>
# include <core_ax/network/Socket.hpp>
# include <util/Singleton.hpp>

# include <list>
# include <memory>

namespace smpp_dmplx {

/*
** ��������������� ������ - ������������ ��������� ��� ���������� ���������
** �������� �������� �����/������ �� ������.
*/
class IdleSocketsPool : public smsc::util::Singleton<IdleSocketsPool> {
public:
  typedef std::pair<smsc::core_ax::network::Socket, time_t> Socket_Timeout_pair_t;
  typedef std::list<Socket_Timeout_pair_t> IdleSocketList_t;

  /*
  ** ���������� true - ���� ���� ������, ��� ������� ����� ������� ��������
  ** ����������, ����� ���������� false.
  */
  bool getTimedOutSocketsList(IdleSocketList_t& timedOutSocketlist);

  /*
  ** �������� ����� � ��� �������, ��������� ���������� �������� �����/������.
  */
  void insertWaitingSocket(const smsc::core_ax::network::Socket& socket, time_t socketTimeoutValue);

  /*
  ** ������� �����, �� ������� ����������� �������� �����/������, �� ����
  */
  void removeActiveSocket(const smsc::core_ax::network::Socket& socket);

private:
  IdleSocketList_t _idle_socket_list;
};

}
#endif
