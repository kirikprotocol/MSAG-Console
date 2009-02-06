#include "SMPP_BindRequest.hpp"
#include "SMPP_Helper.hpp"
#include <sstream>

namespace smpp_dmplx {

SMPP_BindRequest::SMPP_BindRequest(uint32_t msgCode) : SMPP_message(msgCode) {}

SMPP_BindRequest::~SMPP_BindRequest() {}

std::string
SMPP_BindRequest::getSystemId() const
{
  return _systemId;
}

void
SMPP_BindRequest::setSystemId(const std::string& arg)
{
  _systemId = arg; 
}

std::string
SMPP_BindRequest::getPassword() const
{
  return _password;
}

void
SMPP_BindRequest::setPassword(const std::string& arg)
{
  _password = arg;
}

std::auto_ptr<BufferedOutputStream>
SMPP_BindRequest::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(SMPP_message::marshal());
  SMPP_Helper::writeCString(*buf, _systemId, 16);
  SMPP_Helper::writeCString(*buf, _password, 9);
  buf->writeOpaqueData(_unimportantDataBuf);

  return buf;
}

void
SMPP_BindRequest::unmarshal(BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
  // systemId max size 16
  SMPP_Helper::readCString(buf, _systemId, 16);
  // password max size 9
  SMPP_Helper::readCString(buf, _password, 9);
  buf.readOpaqueData(_unimportantDataBuf, getCommandLength() - SMPP_HEADER_SZ - (_systemId.size()+1) - (_password.size()+1));
}

std::string
SMPP_BindRequest::toString() const
{
  std::ostringstream messageDumpBuf;
  messageDumpBuf << SMPP_message::toString()
                 << ",systemId=" << _systemId
                 << ",password=" << _password;
  if ( !_unimportantDataBuf.empty() )
    messageDumpBuf << ",restData=0x" << SMPP_Helper::hexdmp(_unimportantDataBuf);

  return messageDumpBuf.str();
}

}
