#ifndef __SMSC_UTIL_COMM_COMP_RAWTRANSPORTPACKETIFACE_HPP__
# define __SMSC_UTIL_COMM_COMP_RAWTRANSPORTPACKETIFACE_HPP__ 1

# include <core/network/Socket.hpp>
# include <vector>

namespace smsc {
namespace util {
namespace comm_comp {

/*
** ��������� ��� ������������� ���������� "����� ������ � ��������� �����, ������������ �� ����". 
** ������ ������ ����� ������������ ������������ ������� �� ������. ����� ����������� ����� ��������:
** ����� ������ � ���� ������. ������ ������������� ����� ������ � ��� ������ ����������� � �������
** readDataLength, readRawData ����������� �������.
*/
class RawTransportPacketIface {
public:
  virtual ~RawTransportPacketIface() {}

  virtual bool haveReadLenght() const = 0;
  virtual void readDataLength(smsc::core::network::Socket& readySocket) = 0;

  virtual bool haveReadRawData() const = 0;
  virtual void readRawData(smsc::core::network::Socket& readySocket) = 0;

  virtual void writeDataLength(smsc::core::network::Socket& readySocket) = 0;
  virtual void writeRawData(smsc::core::network::Socket& readySocket) = 0;
  virtual bool isPacketWriteComplete() const = 0;

  virtual const std::vector<uint8_t>& getRawData() const = 0;
};

}}}

#endif
