#include <memory>
#include <xercesc/dom/DOM.hpp>

#include "TestConfig.h"
#include "logger/Logger.h"
#include "util/debug.h"
#include "util/findConfigFile.h"
#include "util/xml/DOMTreeReader.h"
#include "util/xml/init.h"
// #include "util/xml/utilFunctions.h"

namespace scag {
namespace config {

    using namespace smsc::util::xml;

    /// gratefully stolen from ConfigManager.cpp
    void TestConfig::init() throw (ConfigException)
    {
        initXerces();
        DOMTreeReader reader;

        const char* cfgFile = smsc::util::findConfigFile("config.xml");
        char* filename = new char[strlen(cfgFile) + 1];
        std::strcpy(filename, cfgFile);
        std::auto_ptr<char> config_filename(filename);

        DOMDocument *document = reader.read( config_filename.get() );
        if (document && document->getDocumentElement())
        {
            DOMElement *elem = document->getDocumentElement();
            __trace__("config readed");
            cfg_.reset( new Config(*elem) );
            __trace2__("parsed %u ints, %u booleans, %u strings\n",
                       cfg_->intParams.GetCount(),
                       cfg_->boolParams.GetCount(),
                       cfg_->strParams.GetCount());
        } else {
            throw ConfigException("Parse result is null");
        }
    }

} // namespace config
} // namespace scag
