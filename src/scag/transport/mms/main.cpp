#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <string>
#include <vector>

#include <readline/readline.h>
#include <readline/history.h>

#include "sms/sms.h"
#include "util/xml/init.h"
#include "core/buffers/File.hpp"
#include "scag/lcm/LongCallManager.h"
#include "util/Exception.hpp"
#include "util/config/ConfigView.h"
#include "logger/Logger.h"


#include "util/findConfigFile.h"

#include "Managers.h"
#include "XMLHandlers.h"
#include "MmsProcessor.h"
#include "util.h"

using namespace scag::exceptions;
using std::string;
using std::exception;
using namespace smsc::sms;
using scag::lcm::LongCallManager;
using smsc::util::Exception;
using scag::config::ConfigManager;
using smsc::logger::Logger;

XERCES_CPP_NAMESPACE_USE

void init() {

  Logger::Init();
  smsc::util::xml::initXerces();

  Logger *log = Logger::getInstance("mms.init");

  smsc_log_info(log, "MMS start initialisation...");
  try {
    ConfigManager::Init();
  } catch (const Exception &e) {
    smsc_log_error(log, "Exception during initialization of ConfigManager: %s", e.what());
    throw Exception("Exception during initialization of ConfigManager: %s", e.what());
  }
  ConfigManager & cfg = ConfigManager::Instance();
  smsc_log_debug(log, "got ConfigManager instance");
  try {
    LongCallManager::Init(cfg.getLongCallManConfig());
  } catch(...) {
    smsc_log_error(log, "Exception during initialization of LongCallManager");
    throw Exception("Exception during initialization of LongCallManager");
  }

  try {
    smsc_log_info(log, "Mms Manager is starting");

    scag::transport::mms::MmsProcessor::Init("./conf");
    scag::transport::mms::MmsProcessor& mp = scag::transport::mms::MmsProcessor::Instance();
    scag::transport::mms::MmsManager::Init(mp, cfg.getMmsManConfig(),
                                            smsc::util::findConfigFile("./conf/mms.xml"));

    smsc_log_info(log, "Mms Manager started");
  } catch(const Exception& e){

    smsc_log_error(log, "Exception during initialization of MmsManager: %s", e.what());
    throw Exception("Exception during initialization of MmsManager: %s", e.what());

  } catch (const XMLException& e) {

    scag::transport::mms::StrX msg(e.getMessage());
    smsc_log_error(log, "Exception during initialization of MmsManager: %s", msg.localForm());
    throw Exception("Exception during initialization of MmsManager: %s", msg.localForm());

  } catch (...) {

    smsc_log_error(log, "Exception during initialization of MmsManager: unknown error");
    throw Exception("Exception during initialization of MmsManager: unknown error");

  }
}

void shutdown() {
  __trace__("shutting down");
  LongCallManager::shutdown();
  scag::transport::mms::MmsManager::Instance().shutdown();
}

int main() {
  try {
    init();
    string exit_cmd("exit");
    for (;;) {
      char* buf = NULL;
      buf = readline("exit >");
      if (!buf) {
        break;
      }
      if (std::strncmp(buf, exit_cmd.c_str(), exit_cmd.size()) == 0) {
        free(buf);
        break;
      }
      free(buf);
    }
    printf("\nshutdown\n");
    shutdown();
  } catch (const Exception& e) {
    printf("%s\n", e.what());
  }
  return 1;
}