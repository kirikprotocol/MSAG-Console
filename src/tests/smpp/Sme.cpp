#include <sme/SmppBase.hpp>
#include <util/smstext.h>
#include "Sme.hpp"

namespace smsc {
	namespace test {
		namespace smpp {

			SmeFactory::GenericSmeHandlerMap SmeFactory::handlers;

			GenericSmeHandler SmeFactory::createSme(ContextHandler ctx) {
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

			log4cpp::Category& BasicSme::log = smsc::test::util::logger.getLog("smsc.test.util.ContextConfigurator");

			BasicSme::Registrator BasicSme::reg;

			void BasicSme::init(ContextHandler ctx) {
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

			class BasicListener : public smsc::sme::SmppPduEventListener {
        smsc::sme::SmppTransmitter* transmitter;
			public:
				void setTransmitter(smsc::sme::SmppTransmitter* transmitter) {
					this->transmitter = transmitter;
				}

				void handleEvent(smsc::smpp::SmppHeader *pdu) {
					std::cout << "Received PDU" << std::endl;

					if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM  ||
						 pdu->get_commandId() == smsc::smpp::SmppCommandSet::DATA_SM)
					{
						char buf[256];
						smsc::sms::SMS s;
						fetchSmsFromSmppPdu((smsc::smpp::PduXSm*)pdu,&s);
						s.getOriginatingAddress().toString(buf,sizeof(buf));
						printf("\n==========\nFrom:%s\n",buf);
						s.getDestinationAddress().toString(buf,sizeof(buf));
						printf("To:%s\n",buf);
						printf("DCS:%d\n",s.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING));
						printf("UMR:%d\n",s.getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE));
						if(smsc::util::getPduText((smsc::smpp::PduXSm*)pdu,buf,sizeof(buf))==-1)
						{
							int sz=((smsc::smpp::PduXSm*)pdu)->optional.size_messagePayload();
							char *data=new char[sz+1];
							if(smsc::util::getPduText((smsc::smpp::PduXSm*)pdu,buf,sizeof(buf))==-1)
							{
								printf("Message(payload):%s\n",data);
							}else
							{
								printf("Error: faield to retrieve message");
							}
							delete [] data;
						}else
						{
							printf("Message:%s\n",buf);
						}
						printf("==========\n");
						fflush(stdout);
						if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM)
						{
							smsc::smpp::PduDeliverySmResp resp;
							resp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DELIVERY_SM_RESP);
							resp.set_messageId("");
							resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
							transmitter->sendDeliverySmResp(resp);
						}else
						{
							smsc::smpp::PduDataSmResp resp;
							resp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DATA_SM_RESP);
							resp.set_messageId("");
							resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
							transmitter->sendDataSmResp(resp);
						}
					}else
					if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::SUBMIT_SM_RESP)
					{
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

			void BasicSme::bind() {
				try {
					BasicListener lst;
					smsc::sme::SmppSession session(cfg, &lst);
					smsc::sme::SmppTransmitter *tr = session.getSyncTransmitter();
					lst.setTransmitter(tr);
					session.connect();
					for(;;) {
					}
					session.close();
				} catch (std::exception &ex) {
					log.error("Error when binding: %s", ex.what());
					std::cout << ex.what() << std::endl;
				}
			}

		}	//namespace smpp
	}	//namespace test
}	//namespace smsc
