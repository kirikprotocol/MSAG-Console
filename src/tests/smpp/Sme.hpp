#ifndef ___SME_BASE_CLASSES___
#define ___SME_BASE_CLASSES___

#include <inttypes.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <tests/util/Configurator.hpp>
#include <tests/util/Event.hpp>
#include "SmppSession.hpp"
#include "ResponseQueue.hpp"
#include "PduQueue.hpp"

namespace smsc {
	namespace test {
		namespace smpp {

			using smsc::test::util::ContextHandler;
			using std::string;

			class SmeConfigurationException : public std::runtime_error {
			public:
				SmeConfigurationException(const char *const message) : std::runtime_error(message) {
				}
				SmeConfigurationException(string message) : std::runtime_error(message.c_str()) {
				}
				SmeConfigurationException() : std::runtime_error("Unexpected error when configuring Sme") {
				}
			};

			class IllegalSmeOperation : public std::runtime_error {
			public:
				IllegalSmeOperation(const char *const message) 
				: std::runtime_error(message) {
				}

				IllegalSmeOperation(string message) 
				: std::runtime_error(message.c_str()) {
				}
			};

			class SmppException : public std::runtime_error {
				const int errorCode;
			public:
				SmppException(const char *const message, int errorCode) 
				: std::runtime_error(message), errorCode(errorCode) {
				}

				SmppException(string message, int errorCode) 
				: std::runtime_error(message.c_str()), errorCode(errorCode) {
				}

				int getErrorCode() const {
					return errorCode;
				}
			};

			class PduListenerException : public SmppException {
			public:
				PduListenerException(const char *const message, int errorCode) 
				: SmppException(message, errorCode){
				}

				PduListenerException(string message, int errorCode) 
				: SmppException(message.c_str(), errorCode) {
				}
			};

			class DeafSmppListener : public smsc::sme::SmppPduEventListener, public smsc::test::util::ProtectedCopy {
				int errorCode;
				bool is_error;
			public:
				DeafSmppListener() : errorCode(0), is_error(false) {}

				void handleEvent(smsc::smpp::SmppHeader *pdu) = 0;

				void handleError(int errorCode) {
					this->errorCode = errorCode;
					is_error = true;
				}

				bool isError() { 
					return is_error; 
				}

				int getErrorCode() {
					return errorCode;
				}

				void releaseError() {
					is_error = false;
				}

				void checkError() throw (PduListenerException) {
					if (is_error) {
						releaseError();
						/*std::cout << "PduListenerException: errorCode = " << errorCode;*/
						std::ostringstream sout;
						sout << "PduListenerException: errorCode = " << errorCode;
						throw PduListenerException(sout.str(), errorCode);
					}
				}
			};

			class SmppListener : public DeafSmppListener {
				void handleEvent(smsc::smpp::SmppHeader *pdu) {}
			};

			class GenericSme : public smsc::test::util::ProtectedCopy {
			public:
				// инициализация
				virtual void init(ContextHandler &ctx) throw(SmeConfigurationException) = 0;
				virtual SmeConfig& getConfig() throw() = 0;
				// методы для SmeFactory
				virtual std::string getId() throw() = 0;
				virtual GenericSmeHandler createInstance() = 0;
				// содержимое в виде строки
				virtual std::string toString() throw() = 0;
				// методы SMPP
				virtual SmppSessionHandler getSession() throw() = 0;
				virtual PduListenerHandler getListener() throw() = 0;
				virtual void setListener(PduListenerHandler &listener) throw(IllegalSmeOperation) = 0;
				virtual void connect() throw(SmppException, IllegalSmeOperation) = 0;
				virtual void reconnect() throw(SmppException, IllegalSmeOperation) = 0;
				virtual void close() throw() = 0;
				virtual uint32_t bind(const int bindType) throw(PduListenerException, IllegalSmeOperation) = 0;
				virtual uint32_t unbind() throw(PduListenerException, IllegalSmeOperation) = 0;
				virtual uint32_t sendEquireLink() throw(PduListenerException, IllegalSmeOperation) = 0;
				virtual uint32_t sendPdu(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation) = 0;
				virtual void sendPduAsIs(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation) = 0;
				virtual void sendGenerickNack(smsc::smpp::PduGenericNack& pdu) throw(PduListenerException, IllegalSmeOperation)=0;
				virtual void sendDeliverySmResp(smsc::smpp::PduDeliverySmResp& pdu) throw(PduListenerException, IllegalSmeOperation)=0;				
				virtual void sendDataSmResp(smsc::smpp::PduDataSmResp& pdu) throw(PduListenerException, IllegalSmeOperation)=0;
                //посылает response на данную pdu
                virtual void respondTo(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation)=0;
				// виртуальный деструктор для производных классов
				virtual ~GenericSme() {}
			};

			class SmeFactory {
				typedef std::map<string, GenericSmeHandler> GenericSmeHandlerMap;
				static GenericSmeHandlerMap handlers;
			public:
				static bool registerSme(GenericSmeHandler creator) {
					return handlers.insert(std::make_pair(creator->getId(), creator)).second;
				}
				static bool unregisterSme(std::string id) {
					return handlers.erase(id) == 1;
				}
				static GenericSmeHandler createSme(ContextHandler ctx) throw(SmeConfigurationException);
			};

			class BasicSme : public GenericSme {
				class Registrator {
				public:
					Registrator() {
						if (!SmeFactory::registerSme(new BasicSme())) {
							log.error("SmeError: Can't register smsc::test::smpp::BasicSme in SmeFactory !");
						}
					}
				};
				static Registrator reg;
			protected:
				// параметры ESME
				SmeConfig cfg;
				// listener
				PduListenerHandler listener;
				// SMPP сессия
				SmppSessionHandler session;
				// Состояние (тип SME после bind)
				int bindType;
				// логгер
				static log4cpp::Category& log;
			public:
  				// деструктор закрывает соединение
  				virtual ~BasicSme() {close();}
				// инициализация
				virtual void init(ContextHandler &ctx) throw(SmeConfigurationException);
				virtual SmeConfig& getConfig() throw() {
					return cfg;
				}
				// методы для SmeFactory
				virtual std::string getId() throw() {
					return "smsc::test::smpp::BasicSme";
				}
				virtual GenericSmeHandler createInstance() {
					return new BasicSme();
				}
				// содержимое в виде строки
				virtual std::string toString() throw();
				// методы SMPP
				virtual SmppSessionHandler getSession() throw() {
					return session;
				}
				virtual PduListenerHandler getListener() throw() {
					return listener;
				}
				virtual void setListener(PduListenerHandler &listener) throw(IllegalSmeOperation) {
					if (session == 0) {
						this->listener = listener;
					} else {
						throw IllegalSmeOperation("IllegalSmeOperation: can't set new listener when Sme session is not null");
					}
				}
				virtual void connect() throw(SmppException, IllegalSmeOperation) {
					if(session == 0) {
						if(listener != 0) {
							session = new SmppSession(cfg, listener.getObjectPtr());
							try {
								session->connect();
							} catch (smsc::sme::SmppConnectException &ex) {
								std::ostringstream sout;
								sout << "Smpp connection error #" << ex.getReason() << ", " << ex.getTextReason();
								throw SmppException(sout.str(), ex.getReason());
							}
						} else {
							throw IllegalSmeOperation("IllegalSmeOperation: can't open new session with null listener");
						}
					}
				}
				virtual void reconnect() throw(SmppException, IllegalSmeOperation) {
				  close();
				  connect();
				}
				virtual void close() throw() {
					if(session != 0) {
						session->close();
						session = SmppSessionHandler();
					}
				}
				virtual uint32_t bind(const int bindType) throw(PduListenerException, IllegalSmeOperation);
				virtual uint32_t unbind() throw(PduListenerException, IllegalSmeOperation);
				virtual uint32_t sendEquireLink() throw(PduListenerException, IllegalSmeOperation);
				virtual uint32_t sendPdu(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation);
				virtual void sendPduAsIs(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation);
				virtual void sendGenerickNack(smsc::smpp::PduGenericNack& pdu) throw(PduListenerException, IllegalSmeOperation);
				virtual void sendDeliverySmResp(smsc::smpp::PduDeliverySmResp& pdu) throw(PduListenerException, IllegalSmeOperation);				
				virtual void sendDataSmResp(smsc::smpp::PduDataSmResp& pdu) throw(PduListenerException, IllegalSmeOperation);
                virtual void respondTo(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation);
			};

			class QueuedSme;

			class QueuedSme : public BasicSme {
				class Registrator {
				public:
					Registrator() {
						if (!SmeFactory::registerSme(new QueuedSme())) {
							log.error("SmeError: Can't register smsc::test::smpp::QueuedSme in SmeFactory !");
						}
					}
				};
				static Registrator reg;

                PduQueue pduQueue;
                ResponseQueue responseQueue;

				class InternalSmppListener : public SmppListener {
					QueuedSme *queuedSme;
				public:
					InternalSmppListener(QueuedSme *qsme) {
						queuedSme = qsme;
					}
					void handleEvent(smsc:: smpp:: SmppHeader *pdu) {
						queuedSme->log.debug("QueuedSmeListener: handling pdu");
						bool isResponse = false;
						switch (pdu->get_commandId()) {
						case smsc::smpp::SmppCommandSet::SUBMIT_SM_RESP:
						case smsc::smpp::SmppCommandSet::SUBMIT_MULTI_RESP:
						case smsc::smpp::SmppCommandSet::DATA_SM_RESP:
						case smsc::smpp::SmppCommandSet::QUERY_SM_RESP:
						case smsc::smpp::SmppCommandSet::CANCEL_SM_RESP:
						case smsc::smpp::SmppCommandSet::REPLACE_SM_RESP:
						case smsc::smpp::SmppCommandSet::BIND_TRANCIEVER_RESP:
						case smsc::smpp::SmppCommandSet::BIND_TRANSMITTER_RESP:
						case smsc::smpp::SmppCommandSet::BIND_RECIEVER_RESP:
						case smsc::smpp::SmppCommandSet::UNBIND_RESP:
						case smsc::smpp::SmppCommandSet::ENQUIRE_LINK_RESP:
						  isResponse = true;
						}
                        if(isResponse) { //пришел ответ
                          queuedSme->responseQueue.processResponse(pdu);
                        } else { //пришла pdu
                          queuedSme->pduQueue.processPdu(pdu);
                        }
					}
				};

			public:
				QueuedSme() {
					PduListenerHandler listener = new InternalSmppListener(this);
					setListener(listener);
				}
				virtual std::string getId() throw() {
					return "smsc::test::smpp::QueuedSme";
				}
				virtual GenericSmeHandler createInstance() {
					return new QueuedSme();
				}
				virtual void setListener(PduListenerHandler &listener) throw(IllegalSmeOperation) {
					if (getListener() != 0) {
						throw IllegalSmeOperation("QueuedSme error: QueuedSme doesn't allow changing it's listener");
					} else {
						BasicSme::setListener(listener);
					}
				}

				virtual void sendPduAsIs(smsc::smpp::SmppHeader *pdu) throw(PduListenerException, IllegalSmeOperation) {
                  BasicSme::sendPduAsIs(pdu);
                  responseQueue.registerPdu(pdu->get_sequenceNumber(), pdu->get_commandId());
                }

				virtual PduHandler receive() {
                  return pduQueue.receive();
				}

				virtual PduHandler receive(uint32_t timeout) {
                  return pduQueue.receive(timeout);
				}

				virtual PduHandler receiveResponse(uint32_t sequence) {
                  return responseQueue.receiveResponse(sequence);
				}

				virtual PduHandler receiveResponse(uint32_t sequence, uint32_t timeout) {
                  return responseQueue.receiveResponse(sequence, timeout);
				}

                virtual void receiveSms(smsc::sms::SMS &sms) throw(PduListenerException, IllegalSmeOperation);
                virtual bool receiveSms(uint32_t timeout, smsc::sms::SMS &sms) throw(PduListenerException, IllegalSmeOperation);
                virtual uint32_t sendSubmitSms(smsc::sms::SMS &sms) throw(PduListenerException, IllegalSmeOperation);
                virtual uint32_t sendDataSms(smsc::sms::SMS &sms) throw(PduListenerException, IllegalSmeOperation);
                
                //проверка ответов на запросы
                virtual bool checkResponse(uint32_t sequence, uint32_t timeout);
                virtual bool checkAllResponses();

				/*virtual PduHandler receiveNoWait() {
					PduHandler pdu;
					pduMutex.Lock();
					if (!queue.empty()) {
						pdu = queue.front();
						queue.pop_front();
					}
					pduMutex.Unlock();
					return pdu;
				}

				virtual PduHandler receiveNoWait(int sequence) {
					queueMutex.Lock();
					PduHandler pdu = findPduWithSequence(sequence);
					queueMutex.Unlock();
					return pdu;
				}*/
			};

		}
	}
}

#endif //___SME_BASE_CLASSES___
