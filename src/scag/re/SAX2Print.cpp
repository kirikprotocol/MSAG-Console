#include <scag/re/RuleEngine.h>
//#include <sme/SmppBase.hpp>
//#include "smppgw/gwsme.hpp"
//#include <fstream>
#include "logger/Logger.h"
#include "RuleStatus.h"
#include "SAX2Print.hpp"
#include "scag/transport/smpp/SmppCommand.h"

using scag::re::RuleEngine;
using scag::re::RuleStatus;
using scag::transport::smpp::SmppCommand;

//using smsc::smppgw::GatewaySme;

/*
static const char*              encodingName    = "LATIN1";
static XMLFormatter::UnRepFlags unRepFlags      = XMLFormatter::UnRep_CharRef;
static SAX2XMLReader::ValSchemes valScheme      = SAX2XMLReader::Val_Auto;
static bool					        expandNamespaces= false ;
static bool                     doNamespaces    = true;
static bool                     doSchema        = true;
static bool                     schemaFullChecking = false;
static bool                     namespacePrefixes = false;

*/



int main(int argC, char* argV[])
{
    //smsc::logger::_trace_cat = 0;

    std::string xmlFile = "rules.xml";
    int errorCount;
    int errorCode;
    SmppCommand command;

    RuleEngine * engine = 0;
    smsc::logger::Logger::Init();

    logger = smsc::logger::Logger::getInstance("SCAG.RuleEngine");
    if (!logger) {return 1;}

    engine = new RuleEngine("./rules");

    RuleStatus rs;

    try
    {
        rs = engine->process(1,command);
        char buff[128];
        sprintf(buff,"%s%d","result = ",rs.result);
        smsc_log_debug(logger,buff);
    }
    catch (Exception& e)
    {
        smsc_log_error(logger,e.what());
        //TODO: Disable route
    }
    


    delete engine;
    smsc::logger::Logger::Shutdown();
    return 0;
}

