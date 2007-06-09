#ifndef __SMSC_UTIL_COMM_COMP_RAWTRANSPORTPACKETIFACE_HPP__
# define __SMSC_UTIL_COMM_COMP_RAWTRANSPORTPACKETIFACE_HPP__ 1

# include <core/network/Socket.hpp>
# include <vector>

namespace smsc {
namespace util {
namespace comm_comp {

/*
** Интерфейс для представления абстракции "пакет данных с префиксом длины, передаваемый по сети". 
** Данные пакета могут доставляться транспортным уровнем по частям. Пакет представлен парой значений:
** длина данных и сами данные. Способ представления длины пакета и его данных реализуется в методах
** readDataLength, readRawData производных классов.
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
