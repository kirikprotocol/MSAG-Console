#include <netinet/in.h>
#include <inttypes.h>
#include <util/Exception.hpp>
#include "NetworkExceptions.hpp"
#include "RawTransportPacket.hpp"

namespace smsc {
namespace util {
namespace comm_comp {

RawTransportPacket::RawTransportPacket()
  : _packetLength(0), _bytesWereRead(0), _bytesWereWrote(0), _haveBeenReadLenght(false), 
    _haveBeenReadData(false), _rawPacketDataWereBeenAssigned(false), _lengthHaveBeenWritten(false),
    _rawPacketDataHaveBeenWritten(false) {}

RawTransportPacket::RawTransportPacket(const std::vector<uint8_t>& rawPacketData)
  : _packetLength(rawPacketData.size()), _bytesWereRead(0), _bytesWereWrote(0), _haveBeenReadLenght(false),
    _haveBeenReadData(false), _rawPacketDataWereBeenAssigned(true), _lengthHaveBeenWritten(false),
    _rawPacketDataHaveBeenWritten(false), _packetData(rawPacketData)
{}

bool
RawTransportPacket::haveReadLenght() const
{
  return _haveBeenReadLenght;
}

void
RawTransportPacket::readDataLength(smsc::core::network::Socket& readySocket)
{
  if ( !_haveBeenReadLenght ) {
    int st = readySocket.Read((char*)&_packetLength + _bytesWereRead, sizeof(_packetLength) - _bytesWereRead);
    if ( st < 0 )
      throw SocketException("RawTransportPacket::readDataLength::: got error on socket [%s]", strerror(errno));
    else if ( st == 0 )
      throw EOFException("RawTransportPacket::readDataLength::: got EOF");
    _bytesWereRead += st;
    if ( _bytesWereRead == sizeof(_packetLength) ) {
      _haveBeenReadLenght = true;
      _bytesWereRead = 0;
      _packetLength = ntohl(_packetLength);
      _packetData.assign(_packetLength, 0);
    }
  }
}

bool
RawTransportPacket::haveReadRawData() const
{
  return _haveBeenReadData;
}

void
RawTransportPacket::readRawData(smsc::core::network::Socket& readySocket)
{
  if ( !_haveBeenReadData ) {
    int st = readySocket.Read((char*)&_packetData[0] + _bytesWereRead, _packetLength - _bytesWereRead);
    if ( st < 0 )
      throw SocketException("RawTransportPacket::readRawData::: got error on socket");
    else if ( st == 0 )
      throw EOFException("RawTransportPacket::readRawData::: got EOF");
    _bytesWereRead += st;
    if ( _bytesWereRead == _packetLength ) {
      _haveBeenReadData = true;
      _bytesWereRead = 0;
    }
  }
}

void
RawTransportPacket::writeDataLength(smsc::core::network::Socket& readySocket)
{
  if ( _rawPacketDataWereBeenAssigned && 
       !_lengthHaveBeenWritten ) {
    int st = readySocket.Write((char*)&_packetLength + _bytesWereWrote, sizeof(_packetLength) - _bytesWereWrote);
    if ( st < 0 )
      throw SocketException("RawTransportPacket::writeDataLength::: got error on socket");
    _bytesWereWrote += st;
    if ( _bytesWereWrote == sizeof(_packetLength) ) {
      _lengthHaveBeenWritten = true; _bytesWereWrote = 0;
    }
  }
}

void
RawTransportPacket::writeRawData(smsc::core::network::Socket& readySocket)
{
  if ( _rawPacketDataWereBeenAssigned && 
       !_rawPacketDataHaveBeenWritten ) {
    int st = readySocket.Write((char*)&_packetData[0] + _bytesWereWrote, _packetLength - _bytesWereWrote);
    if ( st < 0 )
      throw SocketException("RawTransportPacket::writeRawData::: got error on socket");
    _bytesWereWrote += st;
    if ( _bytesWereWrote == _packetLength )
      _rawPacketDataHaveBeenWritten = true;
  }
}

bool
RawTransportPacket::isPacketWriteComplete() const
{
  return _lengthHaveBeenWritten && _rawPacketDataHaveBeenWritten;
}

const std::vector<uint8_t>&
RawTransportPacket::getRawData() const
{
  if ( _haveBeenReadData ||
       _rawPacketDataWereBeenAssigned )
    return _packetData;
  else throw smsc::util::CustomException("RawTransportPacket::getRawData::: data haven' been initialized");
}

}}}
