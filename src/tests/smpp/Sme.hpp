#ifndef ___SME_BASE_CLASSES___
#define ___SME_BASE_CLASSES___

#include <tests/util/Configurator.hpp>
#include <sme/SmppBase.hpp>

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

			class DeafListener : public smsc::sme::SmppPduEventListener, public smsc::test::util::ProtectedCopy {
			  int errorCode;
			  bool is_error;
			protected:
			  DeafListener(DeafListener &listener) {}
			  DeafListener& operator = (DeafListener &listener) { return *this; }
			public:
  				DeafListener() : errorCode(0), is_error(false) {}

				void handleEvent(smsc::smpp::SmppHeader *pdu) {
				  std::cout << "DeafListener: Received PDU" << std::endl;
				}

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
				  if(is_error) {
					releaseError();
					std::ostringstream sout;
					sout << "SmppSessionListenerException: errorCode = " << errorCode;
					throw PduListenerException(sout.str(), errorCode);
				  }
				}
			};
			
			// Typedefs
			class GenericSme;
			typedef smsc::test::util::Handler<GenericSme> GenericSmeHandler;
			typedef smsc::test::util::Handler<smsc::sme::SmppSession> SmppSessionHandler;
			typedef smsc::test::util::Handler<DeafListener> PduListenerHandler;

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
				virtual void bind(const int bindType) throw(PduListenerException, IllegalSmeOperation) = 0;
				virtual void unbind() throw(PduListenerException, IllegalSmeOperation) = 0;
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
				  if(!SmeFactory::registerSme(new BasicSme())) {
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
			  // логгер
			  static log4cpp::Category& log;
			public:
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
				if(session == 0) {
				  this->listener = listener;
				} else {
				  throw IllegalSmeOperation("IllegalSmeOperation: can't set new listener when Sme session is not null");
				}
			  }
			  virtual void bind(const int bindType) throw(PduListenerException, IllegalSmeOperation);
			  virtual void unbind() throw(PduListenerException, IllegalSmeOperation);
			};

			class BindSme : public BasicSme {
			  class Registrator {
			  public:
				Registrator() {
				  if(!SmeFactory::registerSme(new BindSme())) {
					log.error("SmeError: Can't register smsc::test::smpp::BindSme in SmeFactory !");
				  }
				}
			  };
			  static Registrator reg;
			  public:
				// методы для SmeFactory
				virtual std::string getId() throw() {
				  return "smsc::test::smpp::BindSme";
				}
				virtual GenericSmeHandler createInstance() {
				  return new BindSme();
				}
				// позволяет посылать bind request в уже забайнденной сессии
				virtual void bind(const int bindType) throw(PduListenerException);
			};
		}
	}
}

#endif //___SME_BASE_CLASSES___
