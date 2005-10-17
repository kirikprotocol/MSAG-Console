static char const ident[] = "$Id$";
#include <assert.h>

#include "vlr.hpp"

namespace smsc  {
namespace inman {
namespace uss {

VLR::VLR( UCHAR_T user_ssn, UCHAR_T vlr_ssn, const char* vlr_addr,UCHAR_T in_ssn, const char* in_addr)
  : logger( Logger::getInstance("smsc.inman.vlr") )
  , session( 0 )
  , dispatcher( 0 )
  , vlraddr (vlr_addr)
  , vlrssn (vlr_ssn)
  , inaddr (in_addr)
  , inssn (in_ssn)
{
  smsc_log_debug( logger, "Create VLR" );

  Factory* factory = Factory::getInstance();
  assert( factory );

  dispatcher = new Dispatcher();

  session = factory->openSession(user_ssn, vlr_ssn, vlr_addr, in_ssn, in_addr );
  assert( session );
}

VLR::VLR(UCHAR_T vlr_ssn, const char* vlr_addr)
  : logger( Logger::getInstance("smsc.inman.vlr") )
  , session( 0 )
  , dispatcher( 0 )
{
  smsc_log_debug( logger, "Create VLR" );

  Factory* factory = Factory::getInstance();
  assert( factory );

  dispatcher = new Dispatcher();

  session = factory->openSession(vlr_ssn, vlr_addr, vlr_ssn, vlr_addr );
  assert( session );
}

VLR::~VLR()
{
  smsc_log_debug( logger, "Release VLR" );
  Factory* factory = Factory::getInstance();
  assert( factory );
  smsc_log_debug( logger, "ReleaseSession" );
  factory->closeSession( session );
  smsc_log_debug( logger, "Delete dispatcher" );
  delete dispatcher;
}

void VLR::make102(const char * who)
{
  sm_single = new USSDSM(session);
  session->registerDialog(sm_single);
  sm_single->makeRequest(who,vlraddr);
}

void VLR::start()
{
  smsc_log_debug( logger, "Start dispatcher" );
  dispatcher->Start();
}

void VLR::stop()
{
  smsc_log_debug( logger, "Stop dispatcher" );
  dispatcher->Stop();
  dispatcher->WaitFor();
}

} // namespace uss
} // namespace inman
} // namespace smsc
