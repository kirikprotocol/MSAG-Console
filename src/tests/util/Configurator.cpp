#include <dirent.h>

#include <xercesc/dom/DOM_Node.hpp>
#include <xercesc/dom/DOM_Text.hpp>
#include <xercesc/dom/DOM_NodeList.hpp>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>

#include "Configurator.hpp"

namespace smsc {
	namespace test {
		namespace util {
			using std::string;

			Logger logger = Logger();

			// инициализация синглетона
			ContextConfiguratorHandler ContextConfigurator::config = new ContextConfigurator();


			std::string Context::toString(string tab, string indent) {
				using std::ostringstream;

				ostringstream sout;

				sout << tab << "Context {\n";
				sout << tab << tab << "name = " << getName();
				sout << tab << tab << "value = " << getValue() << "\n";
				sout << tab << tab << "Attributes ######\n";
				typedef AttributeMap::const_iterator AIter;
				for (AIter itr = attributes.begin(); itr != attributes.end(); ++itr) {
					sout << tab << tab << "name = " << itr->first << ", value = " << itr->second << "\n";
				}
				typedef ContextHandlerMultiMap::const_iterator SIter;
				for (SIter itr = subcontext.begin(); itr != subcontext.end(); ++itr) {
					itr->second->toString(tab+tab, indent);
				}
				sout << tab << "}\n";
				return sout.str();
			}

			Handler<DOMParser> ContextConfigurator::createParser() {
				//XMLPlatformUtils::Initialize();
				smsc::util::xml::initXerces();
				Handler<DOMParser> parser = new DOMParser();
				//parser->setValidationScheme(DOMParser::Val_Always);
				parser->setValidationScheme(DOMParser::Val_Auto);
				parser->setDoNamespaces(false);
				parser->setDoSchema(false);
				parser->setValidationSchemaFullChecking(false);
				//logger.debug("  Creating ErrorReporter");
				smsc::util::xml::DOMErrorLogger *errReporter = new smsc::util::xml::DOMErrorLogger();
				parser->setErrorHandler(errReporter);
				parser->setCreateEntityReferenceNodes(false);
				parser->setToCreateXMLDeclTypeNode(false);
				parser->setEntityResolver(new smsc::util::xml::DtdResolver());
				//logger.debug("Leaving createParser()");

				return parser;
			}

			DOM_Document ContextConfigurator::parse(string filename) {
				try {
					parser->parse(filename.c_str());
					int errorCount = parser->getErrorCount();
					if (errorCount > 0) {
						char exceptionMsg[1024];
						snprintf(exceptionMsg, sizeof(exceptionMsg), "An %d errors occured during parsing \"%s\"", errorCount, filename.c_str());
						throw ConfigurationException(exceptionMsg);
					}
				} catch (const XMLException &e) {
					char * message = DOMString(e.getMessage()).transcode();
					XMLExcepts::Codes code = e.getCode();
					const char *srcFile = e.getSrcFile();
					unsigned int line = e.getSrcLine();
					char exceptionMsg[1024];
					snprintf(exceptionMsg, sizeof(exceptionMsg), "An error occured during parsing \"%s\" at file \"%s\" on line %d. Nested: %d: %s", filename.c_str(), srcFile, line, code, message);
					delete[] message;
					throw ConfigurationException(exceptionMsg);
				} catch (const DOM_DOMException &e) {
					char msg[1024];
					snprintf(msg, sizeof(msg), "A DOM error occured during parsing\"%s\". DOMException code: %i", filename.c_str(), e.code);
					throw ConfigurationException(msg);
				} catch (...) {
					char msg[1024];
					snprintf(msg, sizeof(msg), "An error occured during parsing \"%s\"", filename.c_str());
					throw ConfigurationException(msg);
				}

				return parser->getDocument();
			}

			void ContextConfigurator::loadContext() {
				loadContextFromDir("./");
			}

			void ContextConfigurator::loadContextFromDir(string dirname) {
				log.debug("Loading all XML-files from directory: " + dirname);
				//int x[128];
				//for(int i=0;i<sizeof(x)/sizeof(int);i++)x[i]=12;
				DIR *dirDescr = opendir(dirname.c_str());
				if (dirDescr) {	// каталог найден и успешно открыт
					bool found = false;
					for (dirent *entry; (entry = readdir(dirDescr));) {
						string filename = entry->d_name;
						string::size_type pos = filename.rfind(".xml");
						if ( (pos != string::npos) && (pos == (filename.length() - 4)) ) {
							found = true;
							log.debug("Loading context from XML-file: " + filename);
							loadContext(filename);
						}
					}
					if (!found) {
						log.warn("There are no XML-files to process in directory: " + dirname);
					}
					//fprintf(stderr,"readdir finished!\n");
					closedir(dirDescr);
				} else { // ошибка открытия каталога
					log.error("Error: Can't open directory: " + dirname);
					throw ConfigurationException("Can't open directory: " + dirname);
				}
			}

			void ContextConfigurator::loadContext(string filename) {
				try {
					log.debug("Parsing file " + filename);
					DOM_Document doc = parse(filename);
					DOM_Element root = doc.getDocumentElement();
					log.debug("Loading root context");
					ContextHandler rootContext = loadContext(root);
					log.debug("Adding root context to the map");
					string contextId = rootContext->getAttribute("context");
					log.debug("ContextId = " + contextId);
					if (contextId != "") {
						contexts.insert(std::make_pair(contextId, rootContext));
					} else {
						contexts.insert(std::make_pair(filename, rootContext));
					}
				} catch (ConfigurationException &ex) {
					throw;
				}
			}

			ContextHandler ContextConfigurator::loadContext(DOM_Element el) {
				// получаем tag name
				char* tagName = el.getTagName().transcode();
				ContextHandler ctx = new Context(tagName);
				delete[] tagName;

				// копируем аттрибуты
				DOM_NamedNodeMap attrs = el.getAttributes();
				for (unsigned int i=0; i<attrs.getLength(); i++) {
					DOM_Node attr = attrs.item(i);
					char *attrname = attr.getNodeName().transcode();
					char *attrvalue = attr.getNodeValue().transcode();
					ctx->setAttribute(attrname, attrvalue);
					delete[] attrname;
					delete[] attrvalue;
				}

				// элемент может иметь значение
				/*char* tagValue = el.getNodeValue().transcode();
				ctx->setValue(tagValue);
				delete[] tagValue;*/

				// просматриваем содержимое
				DOM_NodeList subels = el.getChildNodes();
				for (unsigned int i=0; i<subels.getLength(); i++) {
					DOM_Node node = subels.item(i);
					if(node.getNodeType() == DOM_Node::ELEMENT_NODE) {
					  // рекурсивно добавляем subcontext
					  //DOM_Element &subel = *(DOM_Element*) (&node);
					  DOM_Element &subel = static_cast<DOM_Element&> (node);
					  ContextHandler subCtx = loadContext(subel);
					  ctx->addSubcontext(subCtx);
					} else if(node.getNodeType() == DOM_Node::TEXT_NODE) {
					  // элемент может иметь значение
					  char* tagValue = node.getNodeValue().transcode();
					  ctx->setValue(tagValue);
					  delete[] tagValue;
					}
					/*switch (node.getNodeType()) {
					  case DOM_Node::ELEMENT_NODE:
						// рекурсивно добавляем subcontext
						//DOM_Element &subel = *(DOM_Element*) (&node);
						DOM_Element &subel = static_cast<DOM_Element&> (node);
						ContextHandler subCtx = loadContext(subel);
						ctx->addSubcontext(subCtx);
						break;
					  case DOM_Node::TEXT_NODE:
						// элемент может иметь значение
						DOM_Text &text = static_cast<DOM_Text&> (node);
						char* tagValue = text.getNodeValue().transcode();
						ctx->setValue(tagValue);
						delete[] tagValue;
						break;
					}*/
				}

				return ctx;
			}

			ContextHandler ContextConfigurator::getContext(string name) {
				map<string, ContextHandler>::iterator itr = contexts.find(name);
				if (itr != contexts.end()) {
					std::pair<string, ContextHandler > pair = *itr;
					return pair.second;
				}

				throw ObjectNotFoundException("ObjectNotFoundException: Can't find root context \"" + name + "\"");
			}
		}
	}
}




