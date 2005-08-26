#include "SAX2Print.hpp"

#include <scag/re/RuleEngine.h>
//#include <sme/SmppBase.hpp>
//#include "smppgw/gwsme.hpp"
//#include <fstream>
#include "logger/Logger.h"
#include "scag/re/RuleStatus.h"
#include "scag/transport/smpp/SmppCommand.h"
#include <sms/sms.h>

using scag::re::RuleEngine;
using scag::re::RuleStatus;
using scag::transport::smpp::SmppCommand;
using namespace smsc::sms;

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

    engine = new RuleEngine("./rules");

    RuleStatus rs;

    try
    {
        rs = engine->process(command);
        char buff[128];
        sprintf(buff,"%s%d","result = ",rs.result);
        smsc_log_debug(logger,buff);
    }
    catch (Exception& e)
    {
        smsc_log_error(logger,"");
        smsc_log_error(logger,std::string("Process aborted: ") + e.what());
        //TODO: Disable route
    }
    


    delete engine;
    smsc::logger::Logger::Shutdown();
    return 0;
}

