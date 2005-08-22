static char const ident[] = "$Id$";
#include "console.hpp"

/////////////////////////// INAP API ///////////////////////////////

#include "inman/inap/factory.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/inap/inap.hpp"
#include "inman/interaction/billing.hpp"

////////////////////////////////////////////////////////////////////

using smsc::inman::inap::Factory;
using smsc::inman::inap::Session;
using smsc::inman::inap::TcapDialog;
using smsc::inman::inap::Inap;
using smsc::inman::interaction::Billing;
using smsc::inman::Console;

using std::cout;
using std::endl;

static const UCHAR_T  DEFAULT_SSN = 146;

static Session* g_pSession = 0;

void open(Console&, const std::vector<std::string>& args)
{
    UCHAR_T SSN = DEFAULT_SSN;
    if( args.size() == 2 ) SSN = atoi( args[1].c_str() );
    Session* pSes = Factory::getInstance()->openSession( SSN );
    if( !pSes )
    {
        cout << "Can't open session with SSN=" << (int)SSN << endl;
    }
    else
    {
        g_pSession = pSes;
        cout << "Opened." << endl;
    }
}

void close(Console&, const std::vector<std::string>& args)
{
    Factory::getInstance()->closeSession( g_pSession );
    g_pSession = NULL;
    cout << "Closed." << endl;
}

void select(Console&, const std::vector<std::string>& args)
{
    UCHAR_T SSN = DEFAULT_SSN;
    if( args.size() == 2 ) SSN = atoi( args[1].c_str() );
    Session* pSes = Factory::getInstance()->findSession( SSN );
    if( !pSes )
    {
        cout << "Session with SSN=" << (int)SSN << " not found." << endl;
    }
    else
    {
        g_pSession = pSes;
        cout << "Selected." << endl;
    }
}

//////////////////////////////////////////////////////////////////////
void mosms(Console&, const std::vector<std::string>& args)
{
  ;
  if( !g_pSession )
  {
      cout << "Session not active. Open session first." << endl;
      return;
  }

  TcapDialog* pDialog = g_pSession->openDialog(0);

  if ( !pDialog )
  {
    cout << "Can't open dialog" << endl;
    return;
  }

  Inap*    inap = new Inap( pDialog );

  Billing* bill = new Billing( inap );

  cout << "Interaction started." << endl;
}

int main(int argc, char** argv)
{
  Console console;
  console.addItem("open", &open);
  console.addItem("close", &close);
  console.addItem("select", &select);
  console.addItem("mosms", &mosms);
  console.run("inman>");
  return(0);
}
