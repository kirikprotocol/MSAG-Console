#include "console.hpp"

/////////////////////////// INAP API ///////////////////////////////

#include "inap/factory.hpp"
#include "inap/session.hpp"
#include "inap/dialog.hpp"

////////////////////////////////////////////////////////////////////

using smsc::inman::inap::Factory;
using smsc::inman::inap::Session;
using smsc::inman::inap::Dialog;
using smsc::inman::Console;
using std::cout;
using std::endl;

static const UCHAR_T  DEFAULT_SSN = 201;

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

int main(int argc, char** argv)
{
  Console console;
  console.addItem("open", &open);
  console.addItem("close", &close);
  console.addItem("select", &select);
  console.run("inman>");
  return(0);
}
