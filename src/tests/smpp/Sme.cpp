#include <sme/SmppBase.hpp>
#include <util/smstext.h>
#include "Sme.hpp"

namespace smsc {
	namespace test {
		namespace smpp {

			///////////////////////////////////////////
			// SmeFactory
			//////////////////////////////////////////

			SmeFactory::GenericSmeHandlerMap SmeFactory::handlers;

			GenericSmeHandler SmeFactory::createSme(ContextHandler ctx) throw(SmeConfigurationException) {
				try {
					string smeId = ctx->getAttribute("class");
					GenericSmeHandlerMap::const_iterator itr = handlers.find(smeId);
					if (itr != handlers.end()) {
						GenericSmeHandler result = itr->second->createInstance();
						result->init(ctx);
						return result;
					} else {
						throw SmeConfigurationException("Error: Can't find a creator for identifier=" + smeId);
					}
				} catch (SmeConfigurationException ex) {
					throw;
				} catch (std::runtime_error ex) {
					throw SmeConfigurationException(ex.what());
				}
			}

			///////////////////////////////////////////
			// BasicSme
			//////////////////////////////////////////
			log4cpp::Category& BasicSme::log = smsc::test::util::logger.getLog("smsc.test.smpp.BasicSme");
			BasicSme::Registrator BasicSme::reg;

			class BasicListener : public smsc::sme::SmppPduEventListener {
				smsc::sme::SmppTransmitter* transmitter;
			public:
				void setTransmitter(smsc::sme::SmppTransmitter* transmitter) {
					this->transmitter = transmitter;
				}

				void handleEvent(smsc::smpp::SmppHeader *pdu) {
					std::cout << "Received PDU" << std::endl;

					if (pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM  ||
							pdu->get_commandId() == smsc::smpp::SmppCommandSet::DATA_SM) {
						char buf[256];
						smsc::sms::SMS s;
						fetchSmsFromSmppPdu((smsc::smpp::PduXSm*)pdu,&s);
						s.getOriginatingAddress().toString(buf,sizeof(buf));
						printf("\n==========\nFrom:%s\n",buf);
						s.getDestinationAddress().toString(buf,sizeof(buf));
						printf("To:%s\n",buf);
						printf("DCS:%d\n",s.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING));
						printf("UMR:%d\n",s.getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE));
						if (smsc::util::getPduText((smsc::smpp::PduXSm*)pdu,buf,sizeof(buf))==-1) {
							int sz=((smsc::smpp::PduXSm*)pdu)->optional.size_messagePayload();
							char *data=new char[sz+1];
							if (smsc::util::getPduText((smsc::smpp::PduXSm*)pdu,buf,sizeof(buf))==-1) {
								printf("Message(payload):%s\n",data);
							} else {
								printf("Error: faield to retrieve message");
							}
							delete [] data;
						} else {
							printf("Message:%s\n",buf);
						}
						printf("==========\n");
						fflush(stdout);
						if (pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM) {
							smsc::smpp::PduDeliverySmResp resp;
							resp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DELIVERY_SM_RESP);
							resp.set_messageId("");
							resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
							transmitter->sendDeliverySmResp(resp);
						} else {
							smsc::smpp::PduDataSmResp resp;
							resp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DATA_SM_RESP);
							resp.set_messageId("");
							resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
							transmitter->sendDataSmResp(resp);
						}
					} else
						if (pdu->get_commandId() == smsc::smpp::SmppCommandSet::SUBMIT_SM_RESP) {
						printf("\nReceived async submit sm resp:%d\n",pdu->get_commandStatus());
					}
					//rl_forced_update_display();
					disposePdu(pdu);

				}
				void handleError(int errorCode)
				{
					std::cout << "Error with errorcode = " << errorCode << std::endl;
				}
			};


			void BasicSme::init(ContextHandler &ctx) throw(SmeConfigurationException) {
				try {
					log.debug("BasicSme::init ***enter***");
					// Общие параметры: хост и порт SMSC
					ContextHandler rootCtx = smsc::test::util::ContextConfigurator::instance()->getContext("smsc");
					log.debug("Root context \"smsc\" has been found");
					ContextHandler subCtx = rootCtx->firstSubcontext("host");
					log.debug("Context \"host\" has been found");
					cfg.host = subCtx->getValue();
					subCtx = rootCtx->firstSubcontext("port");
					log.debug("Context \"port\" has been found");
					cfg.port = atoi(subCtx->getValue().c_str());
					// Параметры конкретной SME (обязательные)
					subCtx = ctx->firstSubcontext("sid");
					log.debug("Context \"sid\" has been found");
					cfg.sid = subCtx->getValue();
					subCtx = ctx->firstSubcontext("password");
					log.debug("Context \"password\" has been found");
					cfg.password = subCtx->getValue();
					subCtx = ctx->firstSubcontext("original-address");
					log.debug("Context \"original-address\" has been found");
					cfg.origAddr = subCtx->getValue();
					// Параметры конкретной SME (необязательные)
					try {
						subCtx = ctx->firstSubcontext("timeout");
						log.debug("Context \"timeout\" has been found");
						cfg.timeOut = atoi(subCtx->getValue().c_str());
					} catch (smsc::test::util::ObjectNotFoundException ex) {
						log.warn(ex.what());
						cfg.timeOut = 1000;
					}
					try {
						subCtx = ctx->firstSubcontext("system-type");
						log.debug("Context \"system-type\" has been found");
						cfg.systemType = subCtx->getValue();
					} catch (smsc::test::util::ObjectNotFoundException ex) {
						log.warn(ex.what());
					}

					log.debug("BasicSme::init ***exit***");
				} catch (std::runtime_error ex) {
					throw SmeConfigurationException(ex.what());
				}
			}

			std::string BasicSme::toString() throw() {
				std::ostringstream sout;
				sout << "### ESME {" << std::endl;
				sout << "  host = " << getConfig().host << std::endl;
				sout << "  port = " << getConfig().port << std::endl;
				sout << "  sid = " << getConfig().sid << std::endl;
				sout << "  password = " << getConfig().password << std::endl;
				sout << "  Orig address = " << getConfig().origAddr << std::endl;
				sout << "  System Type = " << getConfig().systemType << std::endl;
				sout << "  timeout = " << getConfig().timeOut << std::endl;
				sout << "} ### ESME" << std::endl;

				return sout.str();
			}

			void BasicSme::bind(const int bindType) throw(PduListenerException, IllegalSmeOperation) {
				if(session == 0) {
					if(listener != 0) {
						session = new SmppSession(cfg, listener.getObjectPtr());
					} else {
						throw IllegalSmeOperation("IllegalSmeOperation: can't bind new session with null listener");
					}
				}
				try {
					session->connect(bindType);
					listener->checkError();
				} catch (smsc::sme::SmppConnectException ex) {
					std::ostringstream msg;
					msg << "PduListenerException: smsc::sme::SmppConnectException occured\n" << ex.getReason();
					throw PduListenerException(msg.str(), ex.getReason());
				}
			}

			void BasicSme::unbind() throw(PduListenerException, IllegalSmeOperation) {
				if (session != 0) {
					try {
						SmppSessionHandler oldSess = session;
						session = SmppSessionHandler();
						oldSess->close();
						listener->checkError();
					} catch (smsc::sme::SmppConnectException ex) {
						std::ostringstream msg;
						msg << "PduListenerException (non-fatal): smsc::sme::SmppConnectException occured\n" << ex.getReason();
						throw PduListenerException(msg.str(), ex.getReason());
					} catch (PduListenerException ex) {
					  std::ostringstream msg;
					  msg << "PduListenerException (non-fatal): smsc::sme::SmppConnectException occured\n" << ex.getErrorCode();
					  log.info(msg.str());
					  throw;
					}
				} else {
					throw IllegalSmeOperation("IllegalSmeOperation: can't unbind null session");
				}
			}

			uint32_t BasicSme::sendEquireLink() throw(PduListenerException, IllegalSmeOperation) {
			  if(session == 0) {
				throw IllegalSmeOperation("BasicSme Error: Can't send EnquireLink when the session is NULL");
			  }
			  uint32_t sequence = session->getNextSeq();
			  smsc::smpp::PduEnquireLink pdu;
			  pdu.get_header().set_commandId(smsc::smpp::SmppCommandSet::ENQUIRE_LINK);
			  pdu.get_header().set_sequenceNumber(sequence);
			  pdu.get_header().set_commandStatus(0);
			  smsc::sme::SmppTransmitter *atrans = session->getAsyncTransmitter();
			  atrans->sendPdu((smsc::smpp::SmppHeader*)&pdu);
			  listener->checkError();
			  return sequence;
			}

			///////////////////////////////////////////
			// BindSme
			//////////////////////////////////////////
			BindSme::Registrator BindSme::reg;

			void BindSme::bind(const int bindType) throw(PduListenerException) {
				if(session == 0) {
				  BasicSme::bind(bindType);
				} else {//issue Bind PDU
				  smsc::smpp::PduBindTRX pdu;
				  int expectedbindresp;
				  switch (bindType) {
				  case smsc::sme::BindType::Transceiver:
					  pdu.get_header().set_commandId(smsc::smpp::SmppCommandSet::BIND_TRANCIEVER);
					  expectedbindresp=smsc::smpp::SmppCommandSet::BIND_TRANCIEVER_RESP;
					  break;
				  case smsc::sme::BindType::Transmitter:
					  pdu.get_header().set_commandId(smsc::smpp::SmppCommandSet::BIND_TRANSMITTER);
					  expectedbindresp=smsc::smpp::SmppCommandSet::BIND_TRANSMITTER_RESP;
					  break;
				  case smsc::sme::BindType::Receiver:
					  pdu.get_header().set_commandId(smsc::smpp::SmppCommandSet::BIND_RECIEVER);
					  expectedbindresp=smsc::smpp::SmppCommandSet::BIND_RECIEVER_RESP;
					  break;
				  default:
  					  std::ostringstream sout;
					  sout << "BindError: Unknown bind type " << bindType;
					  throw PduListenerException(sout.str(), 0);
				  }
				  pdu.set_systemId(cfg.sid.c_str());
				  pdu.set_password(cfg.password.c_str());
				  pdu.set_systemType(cfg.systemType.c_str());
				  int seq=session->getNextSeq();
				  pdu.get_header().set_sequenceNumber(seq);

				  smsc::sme::SmppTransmitter *strans = session->getSyncTransmitter();

				  smsc::smpp::PduBindTRXResp *resp=(smsc::smpp::PduBindTRXResp*)strans->sendPdu((smsc::smpp::SmppHeader*)&pdu);

				  if (!resp || resp->get_header().get_commandId()!=expectedbindresp ||
						  resp->get_header().get_sequenceNumber()!=pdu.get_header().get_sequenceNumber() ||
						  resp->get_header().get_commandStatus()!=smsc::smpp::SmppStatusSet::ESME_ROK) {
					  /*int reason=!resp?SmppConnectException::Reason::timeout :
							   resp->get_header().get_commandId()!=expectedbindresp ||
							   resp->get_header().get_sequenceNumber()!=pdu.get_header().get_sequenceNumber() ?
							   SmppConnectException::Reason::smppError:
							   resp->get_header().get_commandStatus()==smsc::smpp::SmppStatusSet::ESME_RBINDFAIL?
							   SmppConnectException::Reason::bindFailed:
							   SmppConnectException::Reason::unknown;*/
					  int status = resp->get_header().get_commandStatus();
					  if (resp)disposePdu((smsc::smpp::SmppHeader*)resp);
					  std::ostringstream sout;
					  sout << "An error status has been received: " << status;
					  throw PduListenerException(sout.str(), status);
				  }
				}
			}

			///////////////////////////////////////////
			// QueuedSme
			//////////////////////////////////////////
			QueuedSme::Registrator QueuedSme::reg;

		}	//namespace smpp
	}	//namespace test
}	//namespace smsc
