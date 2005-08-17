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
using smsc::inman::inap::InapFactory;
using smsc::inman::inap::InapProtocol;

using smsc::inman::interaction::Interaction;
using smsc::inman::interaction::InteractionListener;
using smsc::inman::interaction::BillingInteraction; // todo: remove this dependency

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
// This class start interaction in ctor and kill self when it finished

class BillSMS : public InteractionListener
{
	BillingInteraction* billing;

	// Preventing creating objects of this class in stack...
	BillSMS( InapProtocol* prot )
	{
		billing = new BillingInteraction( prot );
		billing->addListener( this );
		billing->start();
	}

public:

	// ...but allow in heap
	static BillSMS* create(InapProtocol* pro)
	{
		return new BillSMS( pro );
	}

	~BillSMS()
	{
		billing->removeListener( this );
		delete billing;
	}

	virtual void finished(const Interaction*, unsigned short status)
	{
		cout << "Complete with status: " << status << endl;
		delete this;
	}
};

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

  InapProtocol* pProtocol = 0;

  if ( !pProtocol )
  {
    cout << "Can't create protocol" << endl;
    return;
  }

  BillSMS* interact = BillSMS::create( pProtocol );

  cout << "Done. Interaction started." << endl;
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
