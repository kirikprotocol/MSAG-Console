#ifndef ___SME_BASE_CLASSES___
#define ___SME_BASE_CLASSES___

#include <inttypes.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <tests/util/Configurator.hpp>
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
			// хэндлер освобождает PDU вызывая метод disposePdu
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
							} catch (smsc::sme::SmppConnectException ex) {
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

				std::list<PduHandler> queue;
				smsc::core::synchronization::Mutex queueMutex;
				smsc::core::synchronization::Event queueEvent;
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
						queuedSme->log.debug("QueuedSmeListener: Mutex.Lock()");
						queuedSme->queueMutex.Lock();
						PduHandler pduHandler(pdu);
						queuedSme->log.debug("QueuedSmeListener: pushing pduHandler into queue");
						queuedSme->queue.push_back(pduHandler);
						int sequence = pduHandler->get_sequenceNumber();
						queuedSme->log.debug("QueuedSmeListener: SequenceNumber = %i", sequence);
						queuedSme->maxSequence = (queuedSme->maxSequence < sequence)?sequence:queuedSme->maxSequence;
						if (queuedSme->sequenceLocks.Exist(sequence)) {
							queuedSme->log.debug("QueuedSmeListener: signaling for sequence waiters");
							MutexEventHandler event = queuedSme->sequenceLocks.Get(sequence);
							event->Signal();
						} else {
							queuedSme->log.debug("QueuedSmeListener: signaling for pdu waiters");
							queuedSme->queueEvent.Signal();
						}
						queuedSme->log.debug("QueuedSmeListener: Mutex.Unlock()");
						queuedSme->queueMutex.Unlock();
					}
					/*void handleError(int errorCode) {
					  queuedSme->log.debug("QueuedSmeListener#handleError(%x)", errorCode);
					}*/
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
					queueMutex.Lock();
					if (queue.empty()) {
						queueMutex.Unlock();
						int status = queueEvent.Wait();
						if (status) {	//какая-то ошибка
							return PduHandler();
						}
						queueMutex.Lock();
					}
					PduHandler pdu = queue.front();
					queue.pop_front();
					queueMutex.Unlock();
					return pdu;
				}

				PduHandler receive(int timeout) {
					queueMutex.Lock();
					if (queue.empty()) {
						queueMutex.Unlock();
						int status = queueEvent.Wait(timeout);
						if (status) {	//вылет по таймауту
							return PduHandler();
						}
						queueMutex.Lock();
					}
					PduHandler pdu = queue.front();
					queue.pop_front();
					queueMutex.Unlock();
					return pdu;
				}

				PduHandler receiveWithSequence(int sequence) {
					queueMutex.Lock();
					PduHandler pdu = findPduWithSequence(sequence);
					if (pdu == 0) {
						if (sequenceLocks.Exist(sequence)) { //кто-то уже ждет
							queueMutex.Unlock();
							return pdu;
						}
						MutexEventHandler event = new smsc::core::synchronization::Event();
						sequenceLocks.Insert(sequence, event);
						queueMutex.Unlock();
						int status = event->Wait();
						sequenceLocks.Delete(sequence);
						if (status) {	//какая-то ошибка
							return pdu;
						}
						queueMutex.Lock();
					}
					pdu = findPduWithSequence(sequence);
					queueMutex.Unlock();

					return pdu;
				}

				PduHandler receiveWithSequence(int sequence, int timeout) {
  					log.debug("receiveWithSequence(%d, %d): --- enter", sequence, timeout);
					queueMutex.Lock();
					PduHandler pdu = findPduWithSequence(sequence);
					if (pdu == 0) {
  						log.debug("receiveWithSequence: the queue doesn't contain requested pdu");
						if (sequenceLocks.Exist(sequence)) { //кто-то уже ждет
  							log.debug("receiveWithSequence: sequenceLocks for sequence=%u is alredy busy", sequence);
							queueMutex.Unlock();
							return pdu;
						}
						MutexEventHandler event = new smsc::core::synchronization::Event();
						log.debug("receiveWithSequence: inserting sequence=%u into sequenceLocks", sequence);
						sequenceLocks.Insert(sequence, event);
						queueMutex.Unlock();
						log.debug("receiveWithSequence: waiting for %u", timeout);
						int status = event->Wait(timeout);
						log.debug("receiveWithSequence: wait_status=%u", status);
						sequenceLocks.Delete(sequence);
						if (status) {	// таймаут
							return pdu;
						}
						queueMutex.Lock();
						pdu = findPduWithSequence(sequence);
					}
					queueMutex.Unlock();
  					log.debug("receiveWithSequence: --- exit");

					return pdu;
				}

				PduHandler receiveNoWait() {
					PduHandler pdu;
					queueMutex.Lock();
					if (!queue.empty()) {
						pdu = queue.front();
						queue.pop_front();
					}
					queueMutex.Unlock();
					return pdu;
				}

				PduHandler receiveNoWait(int sequence) {
					queueMutex.Lock();
					PduHandler pdu = findPduWithSequence(sequence);
					queueMutex.Unlock();
					return pdu;
				}
			private:
				PduHandler findPduWithSequence(int sequence) {
					PduHandler pdu;
					if (!queue.empty() && sequence <= maxSequence) {
					  log.debug("findPduWithSequence: searching pdu with sequence %d", sequence);
					  for (PduHandlerListIterator itr = queue.begin(); itr != queue.end(); itr++) {
						  PduHandler queuedPdu = *itr;
						  log.debug("findPduWithSequence: extracted pdu with sequence %d", queuedPdu->get_sequenceNumber());
						  if (sequence == queuedPdu->get_sequenceNumber()) {
							  log.debug("findPduWithSequence: pdu with sequence %d found", queuedPdu->get_sequenceNumber());
							  pdu = queuedPdu;
							  queue.erase(itr);
							  break;
						  }
					  }
					}
					if(pdu != 0) {
					  log.debug("findPduWithSequence: -- exit %d", pdu->get_sequenceNumber());
					}
					return pdu;
				}
			};

			typedef smsc::test::util::Handler<QueuedSme> QueuedSmeHandler;
		}
	}
}

#endif //___SME_BASE_CLASSES___
