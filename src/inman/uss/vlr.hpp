#ident "$Id$"

#ifndef __SMSC_INMAN_USS_VLR__
#define __SMSC_INMAN_USS_VLR__

#include <map>

#include "inman/uss/ussdsm.hpp"
#include "logger/Logger.h"
#include "inman/inap/dispatcher.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"


using smsc::inman::inap::Dispatcher;
using smsc::inman::inap::Session;
using smsc::inman::inap::Dialog;

using smsc::logger::Logger;

namespace smsc  {
namespace inman {
namespace uss {

class VLR
{
    typedef std::map<int, USSDSM*> USSDSMmap;

  public:

    VLR( UCHAR_T vlr_ssn, const char* vlr_addr);
    VLR( UCHAR_T user_ssn, UCHAR_T vlr_ssn, const char* vlr_addr, UCHAR_T in_ssn, const char* in_addr);
    virtual ~VLR();

    void make102(const char * who);
    virtual void start();
    virtual void stop();

  private:
    const char* vlraddr;
    int   vlrssn;
    const char* inaddr;
    int   inssn;
    USSDSM*     sm_single;
    USSDSMmap   workers;
    Logger*     logger;
    Session*    session;
    Dispatcher* dispatcher;
};

}//namespace uss
}//namespace inman
}//namespace smsc

#endif
