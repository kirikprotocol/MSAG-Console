#ident "$Id$"
// Диалог (транзакция) MAP Supplementary services related services

#ifndef __SMSC_INMAN_USS_USSDSM__
#define __SMSC_INMAN_USS_USSDSM__

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/comp/usscomp.hpp"



namespace smsc {
namespace inman {
namespace uss {

using smsc::inman::usscomp::ProcessUSSRequestRes;
using smsc::inman::usscomp::ProcessUSSRequestArg;
using smsc::inman::inap::Session;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::DialogListener;
using smsc::inman::inap::Invoke;

class USSDRequester
{
  public:
    virtual void ProcessUnstructuredSSRequestConf(ProcessUSSRequestRes* arg) = 0;
};

class USSDResponder
{
  public:
    virtual void ProcessUnstructuredSSRequestReq(ProcessUSSRequestArg* arg) = 0;
};

class USSDSM : public Dialog, public DialogListener, public USSDResponder
{
  public:
    setinaddr(char* inaddr);
    USSDSM(Session* session);
    virtual ~USSDSM();
  public:

    void makeRequest(const char* msisdn, const char* vlraddr);
    USHORT_T handleEndDialog();
    void ProcessUnstructuredSSRequestReq(ProcessUSSRequestArg* arg);

    virtual void onDialogInvoke( Invoke* op ); // DialogListener
};

}//namespace uss
}//namespace inman
}//namespace smsc

#endif
