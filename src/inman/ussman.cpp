static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>

#include "inman/uss/vlr.hpp"
#include "inman/common/console.hpp"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

//#define LOCAL_TESTING

static const UCHAR_T VER_HIGH    = 0;
static const UCHAR_T VER_LOW     = 1;

namespace smsc
{
  namespace inman
  {
    namespace inap
    {
        Logger* inapLogger;
        Logger* tcapLogger;
      }
  }
};

using smsc::inman::uss::VLR;
using smsc::inman::uss::VLR_CFG;
using smsc::inman::common::Console;
using smsc::inman::inap::inapLogger;
using smsc::inman::inap::tcapLogger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;


static VLR* vlr = 0;


static void init_logger()
{
  Logger::Init();
    inapLogger = Logger::getInstance("smsc.inman");
    tcapLogger = Logger::getInstance("smsc.inman.inap.dump");
}

extern "C" static void sighandler( int signal )
{
  assert( vlr );
  vlr->stop();
  delete vlr;
  vlr = 0;
}

struct UssManConfig : public VLR_CFG
{
  public:
  UssManConfig()
  {
    vlr_addr = in_addr = host = NULL;
    vlr_ssn = in_ssn = usr_ssn = port = 0;
  }
  void read(Manager& manager)
  {
    try {
      vlr_addr = manager.getString("vlrAddress");
      vlr_ssn = manager.getInt("vlrSSN");
      smsc_log_info( inapLogger, "VLR : GT=%s,SSN=%d", vlr_addr,vlr_ssn );
    } catch(ConfigException& exc) {
     vlr_addr = 0; vlr_ssn = 0;
     throw ConfigException("vlrAddress or vlrSSN missing");
    }
    try {
      in_addr = manager.getString("inAddress");
      in_ssn = manager.getInt("inSSN");
      smsc_log_info( inapLogger, "IN : GT=%s,SSN=%d", in_addr,in_ssn );
    } catch(ConfigException& exc) {
     in_addr = 0; in_ssn = 0;
     throw ConfigException("inAddress or inSSN missing");
    }
    try {
      usr_ssn = manager.getInt("userSSN");
      smsc_log_info( inapLogger, "USER : SSN=%d", usr_ssn );
    } catch(ConfigException& exc) {
      usr_ssn = 0;
     throw ConfigException("userSSN missing");
    }
#ifndef LOCAL_TESTING
    try {
        host = manager.getString("host");
        port = manager.getInt("port");
        smsc_log_info( inapLogger, "USS client: %s:%d", host, port );
    } catch(ConfigException& exc) {
        host = 0; port = 0;
        throw ConfigException("USS client host or port missing");
    }
#endif /* LOCAL_TESTING */
  }
};

#ifdef LOCAL_TESTING
#include "inman/interaction/ussmessages.hpp"
using smsc::inman::interaction::USSRequestMessage;


void make102req(Console&, const std::vector<std::string> &args)
{
    assert( vlr );
    try {
        USSRequestMessage   req;
        req.setReqId(1);
        req.setDCS(0x0F);
        req.setUSSData((unsigned char*)"*100#", 5);
        req.setMSISDNadr("79139343290");
        vlr->onCommandReceived(NULL, &req);
    } catch(...) {
        throw;
    }
}

static void run_console()
{
    Console console;
    console.addItem( "req", make102req );
    console.run("ussman>");
}
#endif /* LOCAL_TESTING */

/*
 * USSman service entry point. Uses by default config.xml
 * USAGE: ussman [xml_config_file_name]
 */
int main(int argc, char** argv)
{
    char *  cfgFile = (char*)"config.xml";

    init_logger();
    smsc_log_info( inapLogger,"****************************");
    smsc_log_info( inapLogger,"* SIBINCO USS MANAGER v%d.%d *", VER_HIGH, VER_LOW);
    smsc_log_info( inapLogger,"****************************");

    if (argc > 1)
        cfgFile = argv[1];
    smsc_log_info(inapLogger,"* Config file: %s", cfgFile);
    smsc_log_info(inapLogger,"****************************");

    UssManConfig cfg;
    try {
        Manager::init((const char *)cfgFile);
        Manager& manager = Manager::getInstance();
        cfg.read(manager);
    } catch (ConfigException& exc) {
        smsc_log_error(inapLogger, "Configuration invalid: %s. Exiting", exc.what());
        exit(-1);
    }
    try {
        vlr = new VLR(&cfg);
        vlr->start();
        sigset( SIGTERM, sighandler );

#ifndef LOCAL_TESTING
        while( vlr )
            usleep( 1000 * 100 );
#else /* LOCAL_TESTING */
        run_console();
        vlr->stop();
        delete vlr;
#endif /* LOCAL_TESTING */

    } catch(const std::exception& error) {
        smsc_log_fatal(inapLogger, "%s", error.what() );
        fprintf( stderr, "Fatal error: %s\n", error.what() );
        delete vlr;
        exit(1);
    }

    return(0);
}
