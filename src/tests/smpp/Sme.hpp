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

			class GenericSme;
			typedef smsc::test::util::Handler<GenericSme> GenericSmeHandler;

			class GenericSme {
			public:
  				// инициализация
				virtual void init(ContextHandler ctx) = 0;
				virtual SmeConfig& getConfig() = 0;
				// методы для SmeFactory
				virtual std::string getId() = 0;
				virtual GenericSmeHandler createInstance() = 0;
				// методы SMPP
				virtual void bind() = 0;
				// виртуальный деструктор для производных классов
				virtual ~GenericSme() {}
			};

			class SmeFactory {
			  typedef std::map<string, GenericSmeHandler> GenericSmeHandlerMap;
			  static GenericSmeHandlerMap handlers;
			public:
			  static bool registerSme(GenericSmeHandler creator) {
				handlers.insert(std::make_pair(creator->getId(), creator));
			  }
			  static bool unregisterSme(std::string id) {
				return handlers.erase(id) == 1;
			  }
			  static GenericSmeHandler createSme(ContextHandler ctx);
			};

			class BasicSme : public GenericSme {
			  class Registrator {
			  public:
				Registrator() {
				  SmeFactory::registerSme(new BasicSme());
				}
			  };
			  static Registrator reg;
			  SmeConfig cfg;
			protected:
			  static log4cpp::Category& log;
			public:
			  virtual void init(ContextHandler ctx);
			  virtual SmeConfig& getConfig() {
				return cfg;
			  }
			  virtual std::string getId() {
				return "smsc::test::smpp::BasicSme";
			  }
			  virtual GenericSmeHandler createInstance() {
				return new BasicSme();
			  }
			  virtual void bind();
			};
		}
	}
}

#endif //___SME_BASE_CLASSES___
