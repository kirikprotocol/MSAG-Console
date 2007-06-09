#ifndef __SMSC_UTIL_COMM_COMP_RAWTRANSPORTPACKET_HPP__
# define __SMSC_UTIL_COMM_COMP_RAWTRANSPORTPACKET_HPP__ 1

# include <core/network/Socket.hpp>
# include <util/comm_comp/RawTransportPacketIface.hpp>

# include <sys/types.h>
# include <vector>

namespace smsc {
namespace util {
namespace comm_comp {

class RawTransportPacket : public RawTransportPacketIface {
public:
  RawTransportPacket();
  RawTransportPacket(const std::vector<uint8_t>& rawPacketData);

  virtual bool haveReadLenght() const;
  virtual void readDataLength(smsc::core::network::Socket& readySocket);

  virtual bool haveReadRawData() const;
  virtual void readRawData(smsc::core::network::Socket& readySocket);

  virtual void writeDataLength(smsc::core::network::Socket& readySocket);
  virtual void writeRawData(smsc::core::network::Socket& readySocket);
  virtual bool isPacketWriteComplete() const;

  virtual const std::vector<uint8_t>& getRawData() const;
private:
  uint32_t _packetLength, _bytesWereRead, _bytesWereWrote;
  bool _haveBeenReadLenght, _haveBeenReadData, _rawPacketDataWereBeenAssigned;
  bool _lengthHaveBeenWritten, _rawPacketDataHaveBeenWritten;
  std::vector<uint8_t> _packetData;
};

}}}

#endif
