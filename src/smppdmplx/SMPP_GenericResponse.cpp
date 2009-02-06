#include "SMPP_GenericResponse.hpp"
#include "SMPP_Helper.hpp"
#include "SMPP_MessageFactory.hpp"
#include <sstream>

namespace smpp_dmplx {

SMPP_GenericResponse::SMPP_GenericResponse(uint32_t msgCode) : SMPP_message(msgCode) {}

SMPP_GenericResponse::~SMPP_GenericResponse() {}

uint32_t
SMPP_GenericResponse::getCommandId() const
{
  return GENERIC_RESPONSE;
}

std::auto_ptr<BufferedOutputStream>
SMPP_GenericResponse::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(SMPP_message::marshal());
  buf->writeOpaqueData(_unimportantDataBuf);

  return buf;
}

void
SMPP_GenericResponse::unmarshal(BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
  buf.readOpaqueData(_unimportantDataBuf, getCommandLength() - SMPP_HEADER_SZ);
}

std::auto_ptr<SMPP_message>
SMPP_GenericResponse::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_GenericResponse(msgCode));
  message->unmarshal(buf);

  return message;
}

std::string
SMPP_GenericResponse::toString() const
{
  std::ostringstream messageDumpBuf;
  messageDumpBuf << SMPP_message::toString();

  if ( !_unimportantDataBuf.empty() )
    messageDumpBuf << ",restData=0x" << SMPP_Helper::hexdmp(_unimportantDataBuf);

  return messageDumpBuf.str();
}

}
