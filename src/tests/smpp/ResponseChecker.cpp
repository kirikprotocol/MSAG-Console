#include <tests/util/Configurator.hpp>
#include "ResponseChecker.hpp"

namespace smsc {
	namespace test {
		namespace smpp {


			log4cpp::Category& ResponseChecker::log = smsc::test::util::logger.getLog("smsc.test.smpp.ResponseChecker");

			//проверка ответа на запрос
			bool ResponseChecker::checkResponse(uint32_t sequence, uint32_t commandId, QueuedSmeHandler sme, uint32_t timeout) {
			  bool res = false;
			  uint32_t expectedCommandId;
			  switch(commandId) {
			  case smsc::smpp::SmppCommandSet::SUBMIT_SM:
				expectedCommandId = smsc::smpp::SmppCommandSet::SUBMIT_SM_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::SUBMIT_MULTI:
				expectedCommandId = smsc::smpp::SmppCommandSet::SUBMIT_MULTI_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::DATA_SM:
				expectedCommandId = smsc::smpp::SmppCommandSet::DATA_SM_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::QUERY_SM:
				expectedCommandId = smsc::smpp::SmppCommandSet::QUERY_SM_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::CANCEL_SM:
				expectedCommandId = smsc::smpp::SmppCommandSet::CANCEL_SM_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::REPLACE_SM:
				expectedCommandId = smsc::smpp::SmppCommandSet::REPLACE_SM_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::BIND_TRANCIEVER:
				expectedCommandId = smsc::smpp::SmppCommandSet::BIND_TRANCIEVER_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::BIND_TRANSMITTER:
				expectedCommandId = smsc::smpp::SmppCommandSet::BIND_TRANSMITTER_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::BIND_RECIEVER:
				expectedCommandId = smsc::smpp::SmppCommandSet::BIND_RECIEVER_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::UNBIND:
				expectedCommandId = smsc::smpp::SmppCommandSet::UNBIND_RESP;
				break;
			  case smsc::smpp::SmppCommandSet::ENQUIRE_LINK:
				expectedCommandId = smsc::smpp::SmppCommandSet::ENQUIRE_LINK_RESP;
				break;
			  }
			  PduHandler pdu = sme->receiveWithSequence(sequence, timeout);
			  if (pdu != 0) {
				  if (log.isDebugEnabled()) {
					  log.debug("checkResponse: Received response");
					  log.debug("checkResponse: command_id=%x", pdu->get_commandId());
					  log.debug("checkResponse: command_status=%x", pdu->get_commandStatus());
					  log.debug("checkResponse: sequence_number=%u", pdu->get_sequenceNumber());
				  }
				  if (pdu->get_commandId() == expectedCommandId) {
					  if (pdu->get_commandStatus() == smsc::smpp::SmppStatusSet::ESME_ROK) {
						  res = true;
					  } else {
						  log.error("checkResponse: The response with sequence=%u has wrong status=%u", sequence, pdu->get_commandStatus());
					  }
				  } else {
					  log.error("checkResponse: The response with sequence=%u has wrong commandId=%x", sequence, pdu->get_commandId());
				  }
			  } else {
				  log.error("checkResponse: There is no response with sequence=%u", sequence);
			  }

			  return res;
			}

			//проверка ответов на запросы bind
			bool ResponseChecker::checkBind(uint32_t sequence, int bindType, QueuedSmeHandler sme, uint32_t timeout) {
				bool res = false;
				uint32_t expectedCommandId;
				std::string bindTypeMsg;
				switch (bindType) {
				case smsc::sme::BindType::Transceiver:
					expectedCommandId = smsc::smpp::SmppCommandSet::BIND_TRANCIEVER_RESP;
					bindTypeMsg = "Transceiver";
					break;
				case smsc::sme::BindType::Transmitter:
					expectedCommandId = smsc::smpp::SmppCommandSet::BIND_TRANSMITTER_RESP;
					bindTypeMsg = "Transmitter";
					break;
				case smsc::sme::BindType::Receiver:
					expectedCommandId = smsc::smpp::SmppCommandSet::BIND_RECIEVER_RESP;
					bindTypeMsg = "Receiver";
					break;
				}
				PduHandler pdu = sme->receiveWithSequence(sequence, timeout);
				if (pdu != 0) {
					if (log.isDebugEnabled()) {
						log.debug("checkBind: Received response for binding for %s", bindTypeMsg.c_str());
						log.debug("checkBind: command_id=%x", pdu->get_commandId());
						log.debug("checkBind: command_status=%x", pdu->get_commandStatus());
						log.debug("checkBind: sequence_number=%u", pdu->get_sequenceNumber());
					}
					if (pdu->get_commandId() == expectedCommandId) {
						smsc::smpp::PduBindTRXResp *resp=(smsc::smpp::PduBindTRXResp*)pdu.getObjectPtr();
						log.debug("checkBind: sms_system_id=%s", resp->get_systemId());
						if (pdu->get_commandStatus() == smsc::smpp::SmppStatusSet::ESME_ROK) {
							res = true;
						} else {
							log.error("checkBind: The bind response with sequence=%u has wrong status=%x", sequence, pdu->get_commandStatus());
						}
					} else {
						log.error("checkBind: The response for %s with sequence=%u has wrong commandId", bindTypeMsg.c_str(), sequence);
					}
				} else {
					log.error("checkBind: There is no response for %s with sequence=%u", bindTypeMsg.c_str(), sequence);
				}
				return res;
			}

			//проверка ответов на запросы unbind
			bool ResponseChecker::checkUnbind(uint32_t sequence, QueuedSmeHandler sme, uint32_t timeout) {
				bool res = false;
				PduHandler pdu = sme->receiveWithSequence(sequence, timeout);
				if (pdu != 0) {
					if (log.isDebugEnabled()) {
						log.debug("checkUnbind: Received response for unbinding");
						log.debug("checkUnbind: command_id=%x", pdu->get_commandId());
						log.debug("checkUnbind: command_status=%x", pdu->get_commandStatus());
						log.debug("checkUnbind: sequence_number=%u", pdu->get_sequenceNumber());
					}
					if (pdu->get_commandId() == smsc::smpp::SmppCommandSet::UNBIND_RESP) {
						if (pdu->get_commandStatus() == smsc::smpp::SmppStatusSet::ESME_ROK) {
							res = true;
						} else {
							log.error("checkUnbind: The unbind response with sequence=%u has wrong status=%x", sequence, pdu->get_commandStatus());
						}
					} else {
						log.error("checkUnbind: The unbind response with sequence=%u has wrong commandId", sequence);
					}
				} else {
					log.error("checkUnbind: There is no response for unbind with sequence=%u", sequence);
				}
				return res;
			}

			//проверка ответов на запросы enquire_link
			bool ResponseChecker::checkEnquireLink(uint32_t sequence, QueuedSmeHandler sme, uint32_t timeout) {
				bool res = false;
				PduHandler pdu = sme->receiveWithSequence(sequence, timeout);
				if (pdu != 0) {
					if (log.isDebugEnabled()) {
						log.debug("checkEnquireLink: Received response for enquire_link");
						log.debug("checkEnquireLink: command_id=%x", pdu->get_commandId());
						log.debug("checkEnquireLink: command_status=%x", pdu->get_commandStatus());
						log.debug("checkEnquireLink: sequence_number=%u", pdu->get_sequenceNumber());
					}
					if (pdu->get_commandId() == smsc::smpp::SmppCommandSet::ENQUIRE_LINK_RESP) {
						if (pdu->get_commandStatus() == smsc::smpp::SmppStatusSet::ESME_ROK) {
							res = true;
						} else {
							log.error("checkEnquireLink: The enquire_link response with sequence=%u has wrong status=%x", sequence, pdu->get_commandStatus());
						}
					} else {
						log.error("checkEnquireLink: The enquire_link response with sequence=%u has wrong commandId", sequence);
					}
				} else {
					log.error("checkUnbind: There is no response for enquire_link with sequence=%u", sequence);
				}
				return res;
			}

		}	// namespace smpp
	}	//namespace test
}	// namespace smsc

