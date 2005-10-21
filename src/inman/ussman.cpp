static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>

#include "inman/uss/vlr.hpp"
#include "inman/common/console.hpp"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"


#define LOCAL_TESTING

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
        Logger* dumpLogger;
      }
  }
};

using smsc::inman::uss::VLR;
using smsc::inman::uss::VLR_CFG;
using smsc::inman::common::Console;
using smsc::inman::inap::inapLogger;
using smsc::inman::inap::tcapLogger;
using smsc::inman::inap::dumpLogger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;


static VLR* vlr = 0;

static void init_logger()
{
  Logger::Init();
    inapLogger = Logger::getInstance("smsc.inman");
    tcapLogger = Logger::getInstance("smsc.inman.inap.dump");
    dumpLogger = Logger::getInstance("smsc.inman.inap.dump");
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
    try {
        host = manager.getString("host");
        port = manager.getInt("port");
        smsc_log_info( inapLogger, "USS client: %s:%d", host, port );
    } catch(ConfigException& exc) {
        host = 0; port = 0;
        throw ConfigException("USS client host or port missing");
    }
  }
};

#ifdef LOCAL_TESTING
void make102req(Console&, const std::vector<std::string> &args)
{
  assert( vlr );
  try
  {
    vlr->make102("79139343290");
  }
  catch(...)
  {
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

int main(int argc, char** argv)
{

  init_logger();
  smsc_log_info( inapLogger,"****************************");
  smsc_log_info( inapLogger,"* SIBINCO USS MANAGER v%d.%d *", VER_HIGH, VER_LOW);
  smsc_log_info( inapLogger,"****************************");

  UssManConfig cfg;
  try
  {
    Manager::init("config.xml");
    Manager& manager = Manager::getInstance();
    cfg.read(manager);
  }
  catch (ConfigException& exc)
  {
      smsc_log_error(inapLogger, "Configuration invalid: %s. Exiting", exc.what());
      exit(-1);
  }
  try
  {
#ifndef LOCAL_TESTING
    vlr = new VLR(&cfg);
#else  /* LOCAL_TESTING */
    vlr = new VLR(cfg.usr_ssn, cfg.vlr_ssn, cfg.vlr_addr, cfg.in_ssn, cfg.in_addr);
#endif /* LOCAL_TESTING */
    vlr->start();
    sigset( SIGTERM, sighandler );

#ifdef LOCAL_TESTING
    run_console();
#endif /* LOCAL_TESTING */

  }
  catch(const std::exception& error)
  {
    smsc_log_fatal(inapLogger, "%s", error.what() );
    fprintf( stderr, "Fatal error: %s\n", error.what() );
    delete vlr;
    exit(1);
  }

  return(0);
}
