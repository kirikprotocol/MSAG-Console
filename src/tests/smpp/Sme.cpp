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
				} catch (SmeConfigurationException &ex) {
					throw;
				} catch (std::runtime_error &ex) {
					throw SmeConfigurationException(ex.what());
				}
			}

			///////////////////////////////////////////
			// BasicSme
			//////////////////////////////////////////
			smsc::logger::Logger BasicSme::log = smsc::test::util::logger.getLog("smsc.test.smpp.BasicSme");
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
					// ����� ���������: ���� � ���� SMSC
					ContextHandler rootCtx = smsc::test::util::ContextConfigurator::instance()->getContext("smsc");
					log.debug("Root context \"smsc\" has been found");
					ContextHandler subCtx = rootCtx->firstSubcontext("host");
					log.debug("Context \"host\" has been found");
					cfg.host = subCtx->getValue();
					subCtx = rootCtx->firstSubcontext("port");
					log.debug("Context \"port\" has been found");
					cfg.port = atoi(subCtx->getValue().c_str());
					// ��������� ���������� SME (������������)
					subCtx = ctx->firstSubcontext("sid");
					log.debug("Context \"sid\" has been found");
					cfg.sid = subCtx->getValue();
					subCtx = ctx->firstSubcontext("password");
					log.debug("Context \"password\" has been found");
					cfg.password = subCtx->getValue();
					subCtx = ctx->firstSubcontext("original-address");
					log.debug("Context \"original-address\" has been found");
					cfg.origAddr = subCtx->getValue();
					// ��������� ���������� SME (��������������)
					try {
						subCtx = ctx->firstSubcontext("timeout");
						log.debug("Context \"timeout\" has been found");
						cfg.timeOut = atoi(subCtx->getValue().c_str());
					} catch (smsc::test::util::ObjectNotFoundException &ex) {
						log.warn(ex.what());
						cfg.timeOut = 10000;
					}
					try {
						subCtx = ctx->firstSubcontext("system-type");
						log.debug("Context \"system-type\" has been found");
						cfg.systemType = subCtx->getValue();
					} catch (smsc::test::util::ObjectNotFoundException &ex) {
						log.warn(ex.what());
					}

					log.debug("BasicSme::init ***exit***");
				} catch (std::runtime_error &ex) {
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

			uint32_t BasicSme::bind(const int bindType) throw(PduListenerException, IllegalSmeOperation) {
			  log.debug("BasicSme#bind() enter");
			  this->bindType = bindType;
			  uint32_t sequence;
			  if(session == 0) {
				throw IllegalSmeOperation("IllegalSmeOperation: can't bind null session");
			  }
              smsc::smpp::PduBindTRX pdu;
              switch (bindType) {
              case BindType::Transceiver:
                  pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANCIEVER);
                  break;
              case BindType::Transmitter:
                  pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANSMITTER);
                  break;
              case BindType::Receiver:
                  pdu.get_header().set_commandId(SmppCommandSet::BIND_RECIEVER);
                  break;
              }
              pdu.set_systemId(cfg.sid.c_str());
              pdu.set_password(cfg.password.c_str());
              pdu.set_systemType(cfg.systemType.c_str());
              sequence = session->getNextSeq();
              pdu.get_header().set_sequenceNumber(sequence);

              sendPduAsIs((smsc::smpp::SmppHeader*)&pdu);
			  log.debug("BasicSme#bind() exit");
			  return sequence;
			}

			uint32_t BasicSme::unbind() throw(PduListenerException, IllegalSmeOperation) {
			  log.debug("BasicSme#unbind() enter");
			  uint32_t sequence;
			  if(session == 0) {
				throw IllegalSmeOperation("IllegalSmeOperation: can't unbind null session");
			  }
			  smsc::smpp::PduUnbind pdu;
			  pdu.get_header().set_commandId(SmppCommandSet::UNBIND);
			  pdu.get_header().set_commandStatus(0);
			  sequence = session->getNextSeq();
			  pdu.get_header().set_sequenceNumber(sequence);

              sendPduAsIs((smsc::smpp::SmppHeader*)&pdu);
			  log.debug("BasicSme#unbind() exit");
			  return sequence;
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
              sendPduAsIs((smsc::smpp::SmppHeader*)&pdu);
			  return sequence;
			}

			uint32_t BasicSme::sendPdu(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation) {
			  if(session == 0) {
				throw IllegalSmeOperation("BasicSme Error: Can't send any PDU when the session is NULL");
			  }
			  uint32_t sequence = session->getNextSeq();
			  pdu->set_sequenceNumber(sequence);
			  pdu->set_commandStatus(smsc::smpp::SmppStatusSet::ESME_ROK);
              sendPduAsIs(pdu);
			  return sequence;
			}

			void BasicSme::sendGenerickNack(smsc::smpp::PduGenericNack& pdu) throw(PduListenerException, IllegalSmeOperation) {
			  if(session == 0) {
				throw IllegalSmeOperation("BasicSme Error: Can't send any PDU when the session is NULL");
			  }
			  smsc::sme::SmppTransmitter *atrans = session->getAsyncTransmitter();
			  atrans->sendGenericNack(pdu);
			  listener->checkError();
			}

			void BasicSme::sendDeliverySmResp(smsc::smpp::PduDeliverySmResp& pdu) throw(PduListenerException, IllegalSmeOperation) {
			  if(session == 0) {
				throw IllegalSmeOperation("BasicSme Error: Can't send any PDU when the session is NULL");
			  }
			  smsc::sme::SmppTransmitter *atrans = session->getAsyncTransmitter();
			  atrans->sendDeliverySmResp(pdu);
			  listener->checkError();
			}

			void BasicSme::sendDataSmResp(smsc::smpp::PduDataSmResp& pdu) throw(PduListenerException, IllegalSmeOperation) {
			  if(session == 0) {
				throw IllegalSmeOperation("BasicSme Error: Can't send any PDU when the session is NULL");
			  }
			  smsc::sme::SmppTransmitter *atrans = session->getAsyncTransmitter();
			  atrans->sendDataSmResp(pdu);
			  listener->checkError();
			}

			void BasicSme::sendPduAsIs(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation) {
              log.debug("sendPduAsIs: --- enter");
			  if(session == 0) {
				throw IllegalSmeOperation("BasicSme Error: Can't send any PDU when the session is NULL");
			  }
			  smsc::sme::SmppTransmitter *atrans = session->getAsyncTransmitter();
			  atrans->sendPdu(pdu);
			  listener->checkError();
              log.debug("sendPduAsIs: --- exit");
			}

            void BasicSme::respondTo(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation) {
              switch(pdu->get_commandId()) {
              case smsc::smpp::SmppCommandSet::DELIVERY_SM: {
                  PduDeliverySmResp resp;
                  resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
                  resp.set_messageId("");
                  resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
                  sendDeliverySmResp(resp);
                  break;
                }
              case smsc::smpp::SmppCommandSet::DATA_SM: {
                  PduDataSmResp resp;
                  resp.get_header().set_commandId(SmppCommandSet::DATA_SM_RESP);
                  resp.set_messageId("");
                  resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
                  sendDataSmResp(resp);
                  break;
                }
              default: {
                  std::ostringstream sout;
                  sout << "Couldn't respond to pdu with commandId=" << pdu->get_commandId();
                  throw IllegalSmeOperation(sout.str());
                }
              }
            }

			///////////////////////////////////////////
			// QueuedSme
			//////////////////////////////////////////
			QueuedSme::Registrator QueuedSme::reg;

            void QueuedSme::receiveSms(smsc::sms::SMS &sms) throw(PduListenerException, IllegalSmeOperation) {
              PduHandler pdu;
              for(;;) {
                pdu = receive();
                if(pdu != 0) {
                  if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM ||
                     pdu->get_commandId() == smsc::smpp::SmppCommandSet::DATA_SM) {
                    // send response
                    respondTo(pdu.getObjectPtr());
                    // check PDU received
                    smsc::smpp::PduXSm *pduXSM = (smsc::smpp::PduXSm*) pdu.getObjectPtr();
                    smsc::smpp::fetchSmsFromSmppPdu(pduXSM, &sms);
                    break;
                  }
                }
              }
            }

            bool QueuedSme::receiveSms(uint32_t timeout, smsc::sms::SMS &sms) throw(PduListenerException, IllegalSmeOperation) {
              bool res = false;
              PduHandler pdu;
              for(;;) {
                timestruc_t time;
                clock_gettime(CLOCK_REALTIME,&time);
                pdu = receive(timeout);
                if(pdu != 0) {
                  if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM) {
                      log.debug("receiveSms: received SMS with delivery_sm pdu");
                      // send response
                      respondTo(pdu.getObjectPtr());
                      // check PDU received
                      smsc::smpp::PduXSm *pduXSM = (smsc::smpp::PduXSm*) pdu.getObjectPtr();
                      smsc::smpp::fetchSmsFromSmppPdu(pduXSM, &sms);
                      res = true;
                      break;
                  } else if (pdu->get_commandId() == smsc::smpp::SmppCommandSet::DATA_SM) {
                    log.debug("receiveSms: received SMS with data_sm pdu");
                    // send response
                    respondTo(pdu.getObjectPtr());
                    // check PDU received
                    smsc::smpp::PduDataSm *pduDataSm = (smsc::smpp::PduDataSm*) pdu.getObjectPtr();
                    smsc::smpp::fetchSmsFromDataSmPdu(pduDataSm, &sms);
                    res = true;
                    break;
                  } else {//unexpected pdu
                    log.error("receiveSms: received unexpected PDU");
                    timestruc_t time1;
                    clock_gettime(CLOCK_REALTIME,&time1);
                    uint32_t interval = (time1.tv_sec - time.tv_sec)*1000 + (time1.tv_nsec - time.tv_nsec)/1000000;
                    if(interval < timeout) {
                      timeout -= interval;
                    } else {
                      break;
                    }
                  }
                } else {
                  break;
                }
              }

              return res;
            }

            uint32_t QueuedSme::sendSubmitSms(smsc::sms::SMS &sms) throw(PduListenerException, IllegalSmeOperation) {
              smsc::smpp::PduSubmitSm submit;
              submit.get_header().set_commandId(smsc::smpp::SmppCommandSet::SUBMIT_SM);
              smsc::smpp::fillSmppPduFromSms(&submit, &sms);
              return sendPdu((smsc::smpp::SmppHeader*)&submit);
            }

            uint32_t QueuedSme::sendDataSms(smsc::sms::SMS &sms) throw(PduListenerException, IllegalSmeOperation) {
              smsc::smpp::PduDataSm dsm;
              dsm.get_header().set_commandId(smsc::smpp::SmppCommandSet::DATA_SM);
              smsc::smpp::fillDataSmFromSms(&dsm, &sms);
              return sendPdu((smsc::smpp::SmppHeader*)&dsm);
            }
		
            bool QueuedSme::checkResponse(uint32_t sequence, uint32_t timeout) {
              return responseQueue.checkResponse(sequence, timeout);
            }

            bool QueuedSme::checkAllResponses() {
              return responseQueue.checkAllResponses();
            }

        }	//namespace smpp
	}	//namespace test
}	//namespace smsc
