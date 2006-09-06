#ifndef  __SMPPDMPLX_PENDINGOUTDATAQUEUE_HPP__
# define __SMPPDMPLX_PENDINGOUTDATAQUEUE_HPP__ 1

# include <core_ax/network/Socket.hpp>
# include <deque>
# include <map>
# include "BufferedOutputStream.hpp"
# include <string.h>

namespace smpp_dmplx {

/*
** ����� ������������� ��������� ��� ���������� �������� ������ ��������.
** ������ ����������� ��� �������� ����� ������� scheduleDataForSending.
** ��������������� �������� ������ ����������� � ������ sendScheduledData - 
** ���������� ������, ��������������� � �������� ��� ������, ���������� 
** � ���������. �� ���� ����� sendScheduledData ������������ ����� �������
** ������, ������� ����� ���� ���������� ��� ������������ ������. �.�. ���
** �������� ����� ������ ������, ���������������� ��� ������� ������,
** ����� ������������� ��������� ������� ������ sendScheduledData
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
