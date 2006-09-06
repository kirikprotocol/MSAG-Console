#include <unistd.h>
#include <util/Exception.hpp>
#include "RawMessage.hpp"
#include "SMPP_message.hpp"
#include "NetworkException.hpp"
#include "SMPP_Constants.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

smpp_dmplx::RawMessage::RawMessage() : _haveReadlength(false), _haveReadRawData(false), _bufForRawMessage(0), _bytesWasRead(0) {}

smpp_dmplx::RawMessage::~RawMessage()
{
  if ( _bufForRawMessage )
    delete [] _bufForRawMessage;
}

bool smpp_dmplx::RawMessage::haveReadLenght() const
{
  return _haveReadlength;
}

void smpp_dmplx::RawMessage::readDataLength(int fd)
{
  if ( !_haveReadRawData ) {
    int sz;
    sz = ::read(fd, _rawMessageLen._bufForLength, sizeof(_rawMessageLen._bufForLength) - _bytesWasRead );
    if ( !sz )
      throw EOFException("RawMessage::readDataLength::: got EOF");
    else if ( sz < 0 )
      throw smsc::util::Exception("RawMessage::readDataLength::: call to read failed");

    _bytesWasRead += sz;
    if ( sizeof(_rawMessageLen._bufForLength) == _bytesWasRead ) {
      _haveReadlength = true;
      _rawMessageLen._messageLength = ntohl(_rawMessageLen._messageLength);

      smsc_log_debug(dmplxlog,"RawMessage::readDataLength::: _rawMessageLen._messageLength=%d",_rawMessageLen._messageLength);
      if ( _rawMessageLen._messageLength < SMPP_message::SMPP_HEADER_SZ )
        throw smsc::util::Exception("RawMessage::readDataLength::: wrong message length");
      if ( _rawMessageLen._messageLength > SMPP_message::MAX_SMPP_MESSAGE_SIZE )
        throw smsc::util::Exception("RawMessage::readDataLength::: message length are exceeded max. value");
      _rawMessageLen._messageLength -= sizeof(uint32_t);
      _bytesWasRead = 0; // reset count for read bytes
    }
  }
}

bool smpp_dmplx::RawMessage::haveReadRawData() const
{
  return _haveReadRawData;
}

void smpp_dmplx::RawMessage::readRawData(int fd)
{
  if ( !_haveReadRawData ) {
    int sz;

    if ( !_bufForRawMessage ) {
      if ( !(_bufForRawMessage = new uint8_t [_rawMessageLen._messageLength]) )
        throw smsc::util::Exception("RawMessage::readRawData::: memory allocation was failed");
    }

    sz = ::read(fd, _bufForRawMessage, _rawMessageLen._messageLength - _bytesWasRead );
    if ( !sz )
      throw EOFException("RawMessage::readRawData::: got EOF");
    else if ( sz < 0 )
      throw smsc::util::Exception("RawMessage::readRawData::: call to read failed");

    _bytesWasRead += sz;
    if ( _rawMessageLen._messageLength == _bytesWasRead )
      _haveReadRawData = true;
  }
}

smpp_dmplx::BufferedInputStream
smpp_dmplx::RawMessage::getRawData() const
{
  smsc_log_debug(dmplxlog,"RawMessage::getRawData::: _rawMessageLen._messageLength=%d",_rawMessageLen._messageLength);
  return smpp_dmplx::BufferedInputStream(std::vector<uint8_t>(_bufForRawMessage, _bufForRawMessage+_rawMessageLen._messageLength));
}
