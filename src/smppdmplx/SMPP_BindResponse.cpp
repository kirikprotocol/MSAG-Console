#include "SMPP_BindResponse.hpp"
#include "SMPP_Helper.hpp"
#include <sstream>

smpp_dmplx::SMPP_BindResponse::SMPP_BindResponse(uint32_t msgCode) : SMPP_message(msgCode) {}

smpp_dmplx::SMPP_BindResponse::~SMPP_BindResponse() {}

std::string
smpp_dmplx::SMPP_BindResponse::getSystemId() const
{
  return _systemId;
}

void
smpp_dmplx::SMPP_BindResponse::setSystemId(const std::string& arg)
{
  _systemId = arg;
}

std::auto_ptr<smpp_dmplx::BufferedOutputStream>
smpp_dmplx::SMPP_BindResponse::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(SMPP_message::marshal());
  SMPP_Helper::writeCString(*buf, _systemId, 16);
  buf->writeOpaqueData(unimportantDataBuf);

  return buf;
}

void
smpp_dmplx::SMPP_BindResponse::unmarshal(smpp_dmplx::BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
  // systemId max size 16
  SMPP_Helper::readCString(buf, _systemId, 16);
  buf.readOpaqueData(unimportantDataBuf, getCommandLength() - SMPP_HEADER_SZ - (_systemId.size()+1));
}


std::string
smpp_dmplx::SMPP_BindResponse::toString() const
{
  std::ostringstream messageDumpBuf;
  messageDumpBuf << SMPP_message::toString()
                 << ",systemId=" << _systemId
                 << ",restData=0x" << SMPP_Helper::hexdmp(unimportantDataBuf);

  return messageDumpBuf.str();
}
