#ifndef ___PROPERTIES_CONFIGURATOR___
#define ___PROPERTIES_CONFIGURATOR___

#include <string>
#include <map>
#include <iterator>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <list>
// XML
/*#include <xercesc/dom/DOM_Document.hpp>
#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <xercesc/parsers/DOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
*/
//smsc includes
#include <util/xml/DOMErrorLogger.h>
#include <util/xml/DtdResolver.h>
#include <util/xml/init.h>

// Logger
#include <logger/Logger.h>

#include "Handler.hpp"
#include "ProtectedCopy.hpp"

namespace smsc {
	namespace test {
		namespace util {

			using std::string;
			using std::map;
			using std::multimap;
			using std::iterator;

			class Logger {
			  bool initialized;
			  void init() {
				if(!initialized) {
				  //std::cout << "Initializing logger\n";
				  smsc::logger::Logger::Init();
				  initialized = true;
				}
			  }
			public:
				Logger() {
				  init();
				}
				smsc::logger::Logger getLog(const char * const category) {
  					init();
					//std::cout << "getting logger for " << category << std::endl;
					return smsc::logger::Logger::getInstance(category);
				}
			};

			extern Logger logger;

			class ObjectNotFoundException : public std::runtime_error {
			public:
				ObjectNotFoundException(const char *const message) : std::runtime_error(message) {
				}
				ObjectNotFoundException(string message) : std::runtime_error(message.c_str()) {
				}
				ObjectNotFoundException() : std::runtime_error("ObjectNotFoundException: an object does not exist") {
				}
			};

			class ConfigurationException : public std::runtime_error {
			public:
				ConfigurationException(const char *const message) : std::runtime_error(message) {
				}
				ConfigurationException(string message) : std::runtime_error(message.c_str()) {
				}
				ConfigurationException() : std::runtime_error("Unexpected error during configuration process") {
				}
			};

			// typedefs
			class Context;
			typedef Handler<Context> ContextHandler;
			typedef multimap<string, ContextHandler > ContextHandlerMultiMap;
			typedef ContextHandlerMultiMap::iterator ContextHandlerIterator;
			typedef std::pair<ContextHandlerIterator, ContextHandlerIterator> ContextHandlerIteratorPair;
			typedef map<string, string> AttributeMap;
			typedef std::list<ContextHandler> ContextHandlerList;

			class Context : public ProtectedCopy {
			private:
				string name;
				string value;
				AttributeMap attributes;
				ContextHandlerMultiMap subcontext;
			public:
				Context(const string& name) : name(name), value("") {
				}

				~Context() {
					//printf("Context destructor for %s !!!", getName().c_str());
				}

				string operator () (string attribute) {
					return attributes[attribute];
				}

				string getAttribute(string name) {
					return attributes[name];
				}

				void setAttribute(const string& name, const string& value) {
					attributes.insert(std::make_pair(name, value));
				}

				ContextHandler operator [] (const string& subContextName) {
					return firstSubcontext(subContextName);
				}

				ContextHandler firstSubcontext(const string& subContextName) {
					ContextHandlerMultiMap::iterator itr = subcontext.find(subContextName);
					if (itr != subcontext.end()) {
						std::pair<string, ContextHandler > pair = *itr;
						return pair.second;
					}

					throw ObjectNotFoundException("ObjectNotFoundException: Can't find subcontext \"" + subContextName + "\" in context " + getName());
				}

				ContextHandlerList findSubcontext(const string& subContextName) {
  					ContextHandlerList lst;
					ContextHandlerIteratorPair subContexts = subcontext.equal_range(subContextName);
					for(ContextHandlerIterator itr = subContexts.first; itr != subContexts.second; ++itr) {
						lst.push_back(itr->second);
					}
					return lst;
				}

				void addSubcontext(const ContextHandler& subCtx) {
					subcontext.insert(std::make_pair(subCtx->getName(), subCtx));
				}

				string getName() {
					return name;
				}
				void setName(const string& name) {
					this->name = name;
				}
				string getValue() {
					return value;
				}
				void setValue(const string& value) {
					this->value = value;
				}
				std::string toString() {
					return toString("  ", "  ");
				}
				std::string toString(string tab, string indent);
			};

			class ContextConfigurator;
			typedef Handler<ContextConfigurator> ContextConfiguratorHandler;

			class ContextConfigurator : public ProtectedCopy {
			private:
				static ContextConfiguratorHandler config;
				map<string, ContextHandler> contexts;
				ContextConfigurator() : parser(createParser()), log(smsc::test::util::logger.getLog("smsc.test.util.ContextConfigurator")) {
				  log.debug("ContextConfigurator constructor");
				}
				Handler<DOMParser> parser;
				Handler<DOMParser> createParser();
				DOM_Document parse(string filename);
				ContextHandler loadContext(DOM_Element root);
				smsc::logger::Logger log;
			public:
				static ContextConfiguratorHandler instance() {
					return config;
				}
				void loadContext();
				void loadContext(string filename);
				void loadContextFromDir(string dirname);
				ContextHandler getContext(string name);     
			};
		}
	}
}


#endif //___PROPERTIES_CONFIGURATOR___
