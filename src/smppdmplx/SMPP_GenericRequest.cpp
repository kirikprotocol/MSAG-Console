#include "SMPP_GenericRequest.hpp"
#include "SMPP_Helper.hpp"
#include "SMPP_MessageFactory.hpp"
#include <sstream>

const uint32_t
smpp_dmplx::SMPP_GenericRequest::GENERIC_REQUEST = 0x0FFFFFFF;

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_GenericRequest(0));
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_GenericRequest::SMPP_GenericRequest(uint32_t msgCode) : SMPP_message(msgCode) {}

smpp_dmplx::SMPP_GenericRequest::~SMPP_GenericRequest() {}

bool
smpp_dmplx::SMPP_GenericRequest::checkMessageCodeEquality(uint32_t msgCode) const
{
  if ( !(msgCode & 0x80000000) &&
       msgCode != BIND_RECEIVER &&
       msgCode != BIND_TRANSMITTER &&
       msgCode != BIND_TRANSCEIVER &&
       msgCode != ENQUIRE_LINK && 
       msgCode != UNBIND ) return true;
  else return false;
}

uint32_t
smpp_dmplx::SMPP_GenericRequest::getCommandId() const
{
  return GENERIC_REQUEST;
}


std::auto_ptr<smpp_dmplx::BufferedOutputStream>
smpp_dmplx::SMPP_GenericRequest::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(SMPP_message::marshal());
  buf->writeOpaqueData(unimportantDataBuf);

  return buf;
}

void
smpp_dmplx::SMPP_GenericRequest::unmarshal(smpp_dmplx::BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
  buf.readOpaqueData(unimportantDataBuf, getCommandLength() - SMPP_HEADER_SZ);
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_GenericRequest::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_GenericRequest(msgCode));
  message->unmarshal(buf);

  return message;
}

std::string
smpp_dmplx::SMPP_GenericRequest::toString() const
{
  std::ostringstream messageDumpBuf;
  messageDumpBuf << SMPP_message::toString()
                 << ",restData=0x" << SMPP_Helper::hexdmp(unimportantDataBuf);

  return messageDumpBuf.str();
}
