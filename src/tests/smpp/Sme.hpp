#ifndef ___SME_BASE_CLASSES___
#define ___SME_BASE_CLASSES___

#include <inttypes.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <tests/util/Configurator.hpp>
#include <tests/util/Event.hpp>
#include "SmppSession.hpp"

namespace smsc {
	namespace test {
		namespace smpp {

			using smsc::test::util::ContextHandler;
			using std::string;
			typedef smsc::sme::SmeConfig SmeConfig;

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

			// handler for pdu
			// ������� ����������� PDU ������� ����� disposePdu
			template <class T> class PduDisposePolicy {
			public:
				void dispose(T *pdu) {
					smsc::smpp::disposePdu(pdu);
				}
			};

			typedef smsc::test::util::Handler<smsc::smpp::SmppHeader, smsc::test::util::Exceptioner, PduDisposePolicy> PduHandler;

			class SmppListener : public DeafSmppListener {
				void handleEvent(smsc::smpp::SmppHeader *pdu) {}
			};

			// Typedefs
			class GenericSme;
			typedef smsc::test::util::Handler<GenericSme> GenericSmeHandler;
			typedef smsc::test::util::Handler<SmppSession> SmppSessionHandler;
			typedef smsc::test::util::Handler<SmppListener> PduListenerHandler;

			class GenericSme : public smsc::test::util::ProtectedCopy {
			public:
				// �������������
				virtual void init(ContextHandler &ctx) throw(SmeConfigurationException) = 0;
				virtual SmeConfig& getConfig() throw() = 0;
				// ������ ��� SmeFactory
				virtual std::string getId() throw() = 0;
				virtual GenericSmeHandler createInstance() = 0;
				// ���������� � ���� ������
				virtual std::string toString() throw() = 0;
				// ������ SMPP
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
				// ����������� ���������� ��� ����������� �������
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
				// ��������� ESME
				SmeConfig cfg;
				// listener
				PduListenerHandler listener;
				// SMPP ������
				SmppSessionHandler session;
				// ��������� (��� SME ����� bind)
				int bindType;
				// ������
				static log4cpp::Category& log;
			public:
  				// ���������� ��������� ����������
  				virtual ~BasicSme() {close();}
				// �������������
				virtual void init(ContextHandler &ctx) throw(SmeConfigurationException);
				virtual SmeConfig& getConfig() throw() {
					return cfg;
				}
				// ������ ��� SmeFactory
				virtual std::string getId() throw() {
					return "smsc::test::smpp::BasicSme";
				}
				virtual GenericSmeHandler createInstance() {
					return new BasicSme();
				}
				// ���������� � ���� ������
				virtual std::string toString() throw();
				// ������ SMPP
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

				typedef std::list<PduHandler> PduHandlerList;
				typedef PduHandlerList::iterator PduHandlerListIterator;

				std::list<PduHandler> pduQueue;
                std::list<PduHandler> responseQueue;
				smsc::core::synchronization::Mutex pduMutex;
                smsc::core::synchronization::Mutex responseMutex;
				smsc::test::util::Event pduEvent;
				int maxSequence;
				typedef smsc::test::util::Handler<smsc::core::synchronization::Event> MutexEventHandler;
				smsc::core::buffers::IntHash<MutexEventHandler> sequenceLocks;

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
                        if(isResponse) { //������ �����
                          queuedSme->log.debug("QueuedSmeListener: responseMutex.Lock()");
                          queuedSme->responseMutex.Lock();
                          PduHandler pduHandler(pdu);
                          int sequence = pduHandler->get_sequenceNumber();
                          queuedSme->log.debug("QueuedSmeListener: SequenceNumber = %i", sequence);
                          queuedSme->maxSequence = (queuedSme->maxSequence < sequence)?sequence:queuedSme->maxSequence;
                          if (queuedSme->sequenceLocks.Exist(sequence)) {//��� �����
                              queuedSme->log.debug("QueuedSmeListener: pushing pduHandler into responseQueue");
                              queuedSme->responseQueue.push_back(pduHandler);
                              queuedSme->log.debug("QueuedSmeListener: signaling for sequence waiters");
                              MutexEventHandler event = queuedSme->sequenceLocks.Get(sequence);
                              event->Signal();
                          } else {//��������� ����� �������
                          }
                          queuedSme->log.debug("QueuedSmeListener: responseMutex.Unlock()");
                          queuedSme->responseMutex.Unlock();
                        } else { //������ pdu
                          queuedSme->log.debug("QueuedSmeListener: pduMutex.Lock()");
                          queuedSme->pduMutex.Lock();
                          PduHandler pduHandler(pdu);
                          queuedSme->log.debug("QueuedSmeListener: pushing pduHandler into pduQueue");
                          queuedSme->pduQueue.push_back(pduHandler);
                          queuedSme->log.debug("QueuedSmeListener: signaling for pdu waiters");
                          queuedSme->pduEvent.Signal();
                          queuedSme->log.debug("QueuedSmeListener: pduMutex.Unlock()");
                          queuedSme->pduMutex.Unlock();
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

				PduHandler receive() {
                  int status = pduEvent.Wait();
                  if (status) {	//�����-�� ������
                      return PduHandler();
                  }
                  pduMutex.Lock();
                  PduHandler pdu = pduQueue.front();
                  pduQueue.pop_front();
                  pduMutex.Unlock();
                  return pdu;
				}

				PduHandler receive(int timeout) {
                  int status = pduEvent.Wait(timeout);
                  if (status) {	//����� �� ��������
                      return PduHandler();
                  }
                  pduMutex.Lock();
                  PduHandler pdu = pduQueue.front();
                  pduQueue.pop_front();
                  pduMutex.Unlock();
                  return pdu;
				}

				PduHandler receiveResponse(int sequence) {
					responseMutex.Lock();
					PduHandler pdu = findResponse(sequence);
					if (pdu == 0) {
						if (sequenceLocks.Exist(sequence)) { //���-�� ��� ����
							responseMutex.Unlock();
							return pdu;
						}
						MutexEventHandler event = new smsc::core::synchronization::Event();
						sequenceLocks.Insert(sequence, event);
						responseMutex.Unlock();
						int status = event->Wait();
						responseMutex.Lock();
						sequenceLocks.Delete(sequence);
						if (status) {	//�����-�� ������
							return pdu;
						}
                        pdu = findResponse(sequence);
					}
					responseMutex.Unlock();

					return pdu;
				}

				PduHandler receiveResponse(int sequence, int timeout) {
  					log.debug("receiveResponse(%d, %d): --- enter", sequence, timeout);
					responseMutex.Lock();
					PduHandler pdu = findResponse(sequence);
					if (pdu == 0) {
  						log.debug("receiveResponse: the queue doesn't contain requested pdu");
						if (sequenceLocks.Exist(sequence)) { //���-�� ��� ����
  							log.error("receiveResponse: sequenceLocks for sequence=%u is alredy busy", sequence);
							responseMutex.Unlock();
							return pdu;
						}
						MutexEventHandler event = new smsc::core::synchronization::Event();
						log.debug("receiveResponse: inserting sequence=%u into sequenceLocks", sequence);
						sequenceLocks.Insert(sequence, event);
						responseMutex.Unlock();
						log.debug("receiveResponse: waiting for %u", timeout);
						int status = event->Wait(timeout);
						responseMutex.Lock();
						log.debug("receiveResponse: wait_status=%u", status);
						sequenceLocks.Delete(sequence);
						if (status) {	// �������
							return pdu;
						}
						pdu = findResponse(sequence);
					}
					responseMutex.Unlock();
  					log.debug("receiveResponse: --- exit");

					return pdu;
				}

				/*PduHandler receiveNoWait() {
					PduHandler pdu;
					pduMutex.Lock();
					if (!queue.empty()) {
						pdu = queue.front();
						queue.pop_front();
					}
					pduMutex.Unlock();
					return pdu;
				}

				PduHandler receiveNoWait(int sequence) {
					queueMutex.Lock();
					PduHandler pdu = findPduWithSequence(sequence);
					queueMutex.Unlock();
					return pdu;
				}*/
			private:
				PduHandler findResponse(uint32_t sequence) {
					PduHandler pdu;
					if (!responseQueue.empty() && sequence <= maxSequence) {
					  log.debug("findResponse: searching response with sequence %d", sequence);
					  for (PduHandlerListIterator itr = responseQueue.begin(); itr != responseQueue.end(); itr++) {
						  PduHandler queuedPdu = *itr;
						  log.debug("findResponse: extracted pdu with sequence %d", queuedPdu->get_sequenceNumber());
						  if (sequence == queuedPdu->get_sequenceNumber()) {
							  log.debug("findResponse: pdu with sequence %d found", queuedPdu->get_sequenceNumber());
							  pdu = queuedPdu;
							  responseQueue.erase(itr);
							  break;
						  }
					  }
					}
					if(pdu != 0) {
					  log.debug("findResponse: -- exit %d", pdu->get_sequenceNumber());
					}
					return pdu;
				}
			};

			typedef smsc::test::util::Handler<QueuedSme> QueuedSmeHandler;
		}
	}
}

#endif //___SME_BASE_CLASSES___
