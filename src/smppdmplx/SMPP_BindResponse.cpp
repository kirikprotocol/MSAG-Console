#include "SMPP_BindResponse.hpp"
#include "SMPP_Helper.hpp"
#include <sstream>

namespace smpp_dmplx {

SMPP_BindResponse::SMPP_BindResponse(uint32_t msgCode) : SMPP_message(msgCode) {}

SMPP_BindResponse::~SMPP_BindResponse() {}

std::string
SMPP_BindResponse::getSystemId() const
{
  return _systemId;
}

void
SMPP_BindResponse::setSystemId(const std::string& arg)
{
  _systemId = arg;
}

std::auto_ptr<BufferedOutputStream>
SMPP_BindResponse::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(SMPP_message::marshal());
  SMPP_Helper::writeCString(*buf, _systemId, 16);
  buf->writeOpaqueData(_unimportantDataBuf);

  return buf;
}

void
SMPP_BindResponse::unmarshal(BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
  // systemId max size 16
  SMPP_Helper::readCString(buf, _systemId, 16);
  buf.readOpaqueData(_unimportantDataBuf, getCommandLength() - SMPP_HEADER_SZ - (_systemId.size()+1));
}


std::string
SMPP_BindResponse::toString() const
{
  std::ostringstream messageDumpBuf;
  messageDumpBuf << SMPP_message::toString()
                 << ",systemId=" << _systemId;

  if ( !_unimportantDataBuf.empty() )
    messageDumpBuf << ",restData=0x" << SMPP_Helper::hexdmp(_unimportantDataBuf);

  return messageDumpBuf.str();
}

}
