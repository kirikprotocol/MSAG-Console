#include "RulesEngine.h"


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
    RulesEngine * engine = 0;

    engine = new RulesEngine();

    engine->ParseFile(xmlFile);
    return 0;
}

