#include "SMPP_BindRequest.hpp"
#include "SMPP_Helper.hpp"
#include <sstream>
#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

smpp_dmplx::SMPP_BindRequest::SMPP_BindRequest(uint32_t msgCode) : SMPP_message(msgCode) {}

smpp_dmplx::SMPP_BindRequest::~SMPP_BindRequest() {}

std::string
smpp_dmplx::SMPP_BindRequest::getSystemId() const
{
  return _systemId;
}

void
smpp_dmplx::SMPP_BindRequest::setSystemId(const std::string& arg)
{
  _systemId = arg; 
}

std::string
smpp_dmplx::SMPP_BindRequest::getPassword() const
{
  return _password;
}

void
smpp_dmplx::SMPP_BindRequest::setPassword(const std::string& arg)
{
  _password = arg;
}

std::auto_ptr<smpp_dmplx::BufferedOutputStream>
smpp_dmplx::SMPP_BindRequest::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(SMPP_message::marshal());
  SMPP_Helper::writeCString(*buf, _systemId, 16);
  SMPP_Helper::writeCString(*buf, _password, 9);
  buf->writeOpaqueData(unimportantDataBuf);

  return buf;
}

void
smpp_dmplx::SMPP_BindRequest::unmarshal(smpp_dmplx::BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
  // systemId max size 16
  SMPP_Helper::readCString(buf, _systemId, 16);
  // password max size 9
  SMPP_Helper::readCString(buf, _password, 9);
  buf.readOpaqueData(unimportantDataBuf, getCommandLength() - SMPP_HEADER_SZ - (_systemId.size()+1) - (_password.size()+1));
}

std::string
smpp_dmplx::SMPP_BindRequest::toString() const
{
  std::ostringstream messageDumpBuf;
  messageDumpBuf << SMPP_message::toString()
                 << ",systemId=" << _systemId
                 << ",password=" << _password
                 << ",restData=0x" << SMPP_Helper::hexdmp(unimportantDataBuf);

  return messageDumpBuf.str();
}
