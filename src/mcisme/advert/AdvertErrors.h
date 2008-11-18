//------------------------------------------------------------------------------
//������������ ������
//------------------------------------------------------------------------------
#ifndef MCISME_ADVERT_ADVERTISING_ERRORS
#define MCISME_ADVERT_ADVERTISING_ERRORS

namespace smsc {
namespace mcisme {

enum 
{
  ERR_ADV_TIMEOUT = 1,    // ������� ������� 
  ERR_ADV_SOCKET,         // ������ ������
  ERR_ADV_SOCK_WRITE,     // ������ ������ ������
  ERR_ADV_BANNER_LEN,     // ������ �� ������� ������ max �����
  ERR_ADV_PACKET_TYPE,    // ������������ ��� ������
  ERR_ADV_PACKET_LEN,     // �������� ����� ������
  ERR_ADV_PACKET_MEMBER,  // �������� ������� ������
  ERR_ADV_QUEUE_FULL,     // ������� �������� �����������
  ERR_ADV_NOT_CONNECTED,
  ERR_ADV_OTHER
};

}}

#endif

