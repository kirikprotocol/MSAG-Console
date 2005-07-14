#include <scag/re/RuleEngine.h>
//#include <sme/SmppBase.hpp>
//#include "smppgw/gwsme.hpp"
//#include <fstream>

using scag::re::RuleEngine;
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

    std::string xmlFile = "rules.xml";
    int errorCount;
    int errorCode;
    RuleEngine * engine = 0;

    engine = new RuleEngine("/rules");

    engine->ParseFile(xmlFile);

    return 0;
}

