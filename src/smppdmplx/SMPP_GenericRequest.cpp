#include <sstream>
#include "SMPP_GenericRequest.hpp"
#include "SMPP_Helper.hpp"
#include "SMPP_MessageFactory.hpp"

namespace smpp_dmplx {

SMPP_GenericRequest::SMPP_GenericRequest(uint32_t msgCode) : SMPP_message(msgCode) {}

SMPP_GenericRequest::~SMPP_GenericRequest() {}

uint32_t
SMPP_GenericRequest::getCommandId() const
{
  return GENERIC_REQUEST;
}

std::auto_ptr<BufferedOutputStream>
SMPP_GenericRequest::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(SMPP_message::marshal());
  buf->writeOpaqueData(_unimportantDataBuf);

  return buf;
}

void
SMPP_GenericRequest::unmarshal(BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
  buf.readOpaqueData(_unimportantDataBuf, getCommandLength() - SMPP_HEADER_SZ);
}

std::auto_ptr<SMPP_message>
SMPP_GenericRequest::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_GenericRequest(msgCode));
  message->unmarshal(buf);

  return message;
}

std::string
SMPP_GenericRequest::toString() const
{
  std::ostringstream messageDumpBuf;
  messageDumpBuf << SMPP_message::toString();

  if ( !_unimportantDataBuf.empty() )
    messageDumpBuf << ",restData=0x" << SMPP_Helper::hexdmp(_unimportantDataBuf);

  return messageDumpBuf.str();
}

}
