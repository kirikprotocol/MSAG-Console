#include "SAX2Print.hpp"

#include <scag/re/RuleEngine.h>
#include "logger/Logger.h"
#include "scag/re/RuleStatus.h"
#include "scag/transport/smpp/SmppCommand.h"
#include <sms/sms.h>
#include "scag/sessions/SessionManager.h"
#include "scag/bill/BillingManager.h"

using scag::re::RuleEngine;
using scag::re::RuleStatus;
using scag::transport::smpp::SmppCommand;
using namespace smsc::sms;
using namespace scag::sessions;
using namespace scag::bill;


int main(int argC, char* argV[])
{

    std::string xmlFile = "rules.xml";
    int errorCount;
    int errorCode;
    SMS sms;
    SmppCommand command = SmppCommand::makeDeliverySm(sms,51);

    RuleEngine * engine = 0;
    smsc::logger::Logger::Init();

    logger = smsc::logger::Logger::getInstance("SCAG.RuleEngine");
    if (!logger) {return 1;}

    Session * session = 0;
    SessionManager * sm1;

    try
    {
        SessionManagerConfig smcfg;
        SessionManager::Init(smcfg);

        std::cout<<"session manager inited" << std::endl;
        SessionManager& sm2 = SessionManager::Instance();
        std::cout<<"session manager get instance" << std::endl;
        sm1 = &sm2;
    } catch (...)
    {
        std::cout<<"error creating session manager" << std::endl;
        return 2;
    }
    CSessionKey key;
    session = sm1->newSession(key);

    std::cout<<"session manager get session" << std::endl;

    if (!session) 
    {
        std::cout<<"session is not valid" << std::endl;
        //smsc::logger::Logger::Shutdown();
        std::cout<<"system shutdown" << std::endl;
        return 3;
    }



    engine = new RuleEngine();
    RuleStatus rs;

    BillingManagerConfig billingManagerConfig;

    billingManagerConfig.so_dir = "./bill";
    billingManagerConfig.cfg_dir = "";
    billingManagerConfig.mainActionFactory = engine->getActionFactory();

    BillingManager::Init(billingManagerConfig);
    engine->Init("./rules");

    try
    {
        rs = engine->process(command, *session);
        smsc_log_debug(logger,"result = %d",rs.result);
    }
    catch (Exception& e)
    {
        smsc_log_error(logger,"");
        smsc_log_error(logger,"Process aborted: %s",e.what());
        //TODO: Disable route
    }
    


    delete engine;
    //smsc::logger::Logger::Shutdown();
    return 0;
}

