#include "SMPP_GenericResponse.hpp"
#include "SMPP_Helper.hpp"
#include "SMPP_MessageFactory.hpp"
#include <sstream>

const uint32_t
smpp_dmplx::SMPP_GenericResponse::GENERIC_RESPONSE = 0x8FFFFFFF;

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_GenericResponse(0));
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_GenericResponse::SMPP_GenericResponse(uint32_t msgCode) : SMPP_message(msgCode) {}

smpp_dmplx::SMPP_GenericResponse::~SMPP_GenericResponse() {}

bool
smpp_dmplx::SMPP_GenericResponse::checkMessageCodeEquality(uint32_t msgCode) const
{
  if ( (msgCode & 0x80000000) &&
       msgCode != BIND_RECEIVER_RESP &&
       msgCode != BIND_TRANSMITTER_RESP &&
       msgCode != BIND_TRANSCEIVER_RESP &&
       msgCode != ENQUIRE_LINK_RESP && 
       msgCode != UNBIND_RESP ) return true;
  else return false;
}

uint32_t
smpp_dmplx::SMPP_GenericResponse::getCommandId() const
{
  return GENERIC_RESPONSE;
}

std::auto_ptr<smpp_dmplx::BufferedOutputStream>
smpp_dmplx::SMPP_GenericResponse::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(SMPP_message::marshal());
  buf->writeOpaqueData(unimportantDataBuf);

  return buf;
}

void
smpp_dmplx::SMPP_GenericResponse::unmarshal(smpp_dmplx::BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
  buf.readOpaqueData(unimportantDataBuf, getCommandLength() - SMPP_HEADER_SZ);
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_GenericResponse::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_GenericResponse(msgCode));
  message->unmarshal(buf);

  return message;
}

std::string
smpp_dmplx::SMPP_GenericResponse::toString() const
{
  std::ostringstream messageDumpBuf;
  messageDumpBuf << SMPP_message::toString()
                 << ",restData=0x" << SMPP_Helper::hexdmp(unimportantDataBuf);

  return messageDumpBuf.str();
}
