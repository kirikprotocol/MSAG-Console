#include <sys/types.h>
#include <dirent.h>
#include <cassert>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

// #include <core/buffers/IntHash.hpp>

#include <util/debug.h>
#include <locale.h>

#include "RuleEngine2.h"

#include "scag/re/base/CommandBridge.h"
#include "scag/re/base/Rule2.h"
// #include "scag/re/base/SmppTransportRule.h"
#include "scag/re/base/XMLHandlers2.h"
#include "scag/sessions/base/Operation.h"
#include "util/regexp/RegExp.hpp"


//namespace {
//smsc::core::synchronization::Mutex transportRuleLock;
//}


namespace scag2 {
namespace re {


/*
TransportRule* RuleEngineImpl::getTransportRule( TransportType tt ) const
{
    const unsigned idx = unsigned(tt)-1;
    if ( idx >= maxrule )
        throw SCAGException("getTransportRule: transport=%u is wrong", unsigned(tt) );
    std::auto_ptr< TransportRule >& autr = transportRules_[idx];
    if ( ! autr.get() ) {

        MutexGuard mg(transportRuleLock);
        if ( ! autr.get() ) {
            switch (tt) {
            case SMPP :
                autr.reset( new smpp::SmppTransportRule );
                break;
            default:
                // FIXME: impl
                throw SCAGException("getTransportRule: transport=%u is not impl yet", unsigned(tt) );
                break;
            }
        }
    }
    return autr.get();
}
 */


void RuleEngineImpl::ReadRulesFromDir(TransportType transport, const char * dir)
{
    DIR * pDir = 0;
    dirent * pDirEnt = 0;
    int serviceId = 0;

    pDir = opendir(dir);
    if (!pDir)
    {
        smsc_log_warn(logger,"Invalid directory %s", dir);
        return;
    }

    while (pDir)
    {
        pDirEnt = readdir(pDir);
        if (pDirEnt)
        {
            if (isValidFileName(pDirEnt->d_name, serviceId))
            {
                smsc_log_debug(logger,"Rule ID is %d", serviceId);
                RuleKey key;
                key.transport = transport;
                key.serviceId = serviceId;
                try
                {
                    updateRule(key);
                } catch (SCAGException& e)
                {
                    //smsc_log_error(logger,"%s",e.what());
                }
            }
            else if ((strcmp(pDirEnt->d_name,".")!=0)&&(strcmp(pDirEnt->d_name,"..")!=0))
            {
                smsc_log_info(logger,"Skipped '%s' file: Invalid rule name",pDirEnt->d_name);
            }
        }
        else
            break;
    }

    closedir(pDir);
}


Rule * RuleEngineImpl::ParseFile(const std::string& xmlFile)
{
    int errorCount = 0;
    // int errorCode = 0;

    SAXParser parser;
    XMLBasicHandler handler(factory);

    try
    {
        parser.setValidationScheme(SAXParser::Val_Always);
        parser.setDoSchema(true);
        parser.setValidationSchemaFullChecking(true);
        parser.setDoNamespaces(true);

        parser.setValidateAnnotations(false);

        parser.setValidationConstraintFatal(true);

        parser.setDocumentHandler(&handler);
        parser.setErrorHandler(&handler);


        parser.parse(xmlFile.c_str());
        errorCount = parser.getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
        smsc_log_error(logger,"Terminate parsing Rule: XMLPlatform: OutOfMemoryException");
        throw SCAGException("Terminate parsing Rule: XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        smsc_log_error(logger,"Terminate parsing Rule: An error occurred. Error: %s", msg.localForm());
        throw SCAGException("Terminate parsing Rule: An error occurred. Error: %s", msg.localForm());
    }
    catch (RuleEngineException& e)
    {
        if (e.getLineNumber()>0) smsc_log_error(logger,"Error at line %d: %s",e.getLineNumber(), e.what());
        else smsc_log_error(logger,"Error: %s",e.what());
        throw;
    }
    catch (SCAGException& e)
    {
        smsc_log_error(logger,"Terminate parsing Rule: %s",e.what());
        throw;
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing Rule: unknown fatal error");
        throw SCAGException("Terminate parsing Rule: unknown fatal error");
    }

    //delete parser;

    if (errorCount > 0)
    {
        smsc_log_error(logger,"Error parsing Rule: some errors occured");
        throw SCAGException("Error parsing Rule: some errors occured");
    }

    Rule * rule = handler.ReturnFinalObject();
    return rule;


}
/*
bool RuleEngineImpl::findTransport(const char * name, TransportType& transportType)
{
    TransportType * transportPTR = TransportTypeHash.GetPtr(name);
    if (!transportPTR) return false;

    transportType = *transportPTR;
    return true;

}
*/

bool RuleEngineImpl::isValidFileName(std::string fname, int& serviceId)
{
    if (fname.substr(0,5).compare("rule_")) return false;
    if (fname.substr(fname.size()-4,4).compare(".xml")) return false;

    std::string str = fname.substr(5,fname.size()-5-4);

    if ((str[0] == '0')&&(str.length() > 1)) return false;

    for (unsigned i = 1; i < str.length(); i++)
    {
        if ((str[i] < '0')||(str[i] > '9')) return false;
    }

    serviceId = atoi(str.c_str());

    return (serviceId >= 0);
}

std::string RuleEngineImpl::CreateRuleFileName(const std::string& dir,const RuleKey& key)
{
    char buff[100];

    char * transportkey = 0;
    scag::transport::TransportType value;

    TransportTypeHash.First();
    for (Hash <scag::transport::TransportType>::Iterator it = TransportTypeHash.getIterator(); it.Next(transportkey, value);)
    {
        if (value == key.transport) break;
    }

    sprintf(buff,"%d", key.serviceId);
    std::string result = dir;

    result.append("/");
    result.append(transportkey);
    result.append("/rule_");
    result.append(buff);
    result.append(".xml");

    return result;
}

void RuleEngineImpl::process( SCAGCommand& command, Session& session, RuleStatus& rs )
{
    RulesReference rulesRef = getRules();
    // smsc_log_debug(logger,"Process RuleEngine (total=%u) with serviceId: %d",
    // rulesRef.rules->rules.Count(), command.getServiceId());

    RuleKey key;
    key.serviceId = command.getServiceId();
    key.transport = command.getType();

    Rule* rulePtr;
    do { // fake loop

        if ( session.getLongCallContext().continueExec ) {
            // take rule from action context
            __require__( session.getLongCallContext().getActionContext() );
            rulePtr = session.getLongCallContext().getActionContext()->getRule();
        } else {
            Rule** rp = rulesRef.rules->rules.GetPtr(key);
            rulePtr = rp ? *rp : 0;
        }

        if ( session.isNew( key.serviceId, key.transport ) ) { // fake while

            session.pushInitRuleKey( key.serviceId, key.transport );
            if ( rulePtr ) {

                rulePtr->processSession( session, rs );
                if ( rs.status == STATUS_FAILED ) {
                    session.dropInitRuleKey( key.serviceId, key.transport );
                    // session.setNew( key.serviceId, key.transport, false );
                }
                if ( rs.status != STATUS_OK ) break;
            }
            session.setNew( key.serviceId, key.transport, false );

        }

        /*
         TransportRule* tr = getTransportRule( command.getType() );
         assert( tr );
         TransportRule::Guard g( *tr, command, session, rs );
         try {
         g.init();
         } catch ( SCAGException& e ) {
         smsc_log_error( logger, "TransportRule cannot start/locate operation. Details: %s", e.what() );
         rs.result = smsc::system::Status::SMDELIFERYFAILURE;
         rs.status = STATUS_FAILED;
         return;
         }
         */
        if ( rulePtr ) rulePtr->process( command, session, rs );

        // check if we need to destroy the service
        if ( rs.status == STATUS_OK && 
             session.getLongCallContext().getActionContext() &&
             session.getLongCallContext().getActionContext()->getDestroyService() ) {
            if ( rulePtr ) rulePtr->processSession( session, rs );
            if ( rs.status == STATUS_OK ) 
                session.dropInitRuleKey(key.serviceId, key.transport);
        }
    
    } while ( false );

    if ( rs.status == STATUS_LONG_CALL && rulePtr ) {
        // session.getLongCallContext().continueExec = true;
        session.getLongCallContext().getActionContext()->setRule( *rulePtr );
    }
    // else 
    // smsc_log_debug(logger,"rule for serv=%d, trans=%d not found, ok", key.serviceId, key.transport );
}


void RuleEngineImpl::processSession(Session& session, RuleStatus& rs)
{
    RulesReference rulesRef = getRules();

    Rule* rulePtr = 0;
    while ( true ) {

        RuleKey key;
        if ( ! session.getRuleKey(key.serviceId, key.transport) ) break;

        // isNew may be not set in case of failure from long call
        // assert( ! session.isNew(key.serviceId, key.transport) );
        // make sure 
        if ( session.isNew( key.serviceId, key.transport ) ) {
            smsc_log_warn( logger, "session=%p/%s didn't finish init svc/trans=%d/%d",
                           &session, session.sessionKey().toString().c_str(),
                           int(key.serviceId), int(key.transport) );
            session.dropInitRuleKey(key.serviceId, key.transport);
            continue;
        }

        {
            Rule** rp = rulesRef.rules->rules.GetPtr(key);
            rulePtr = rp ? *rp : 0;
        }

        if (rulePtr) {

            rulePtr->processSession( session, rs );
            if ( rs.status == STATUS_LONG_CALL ) {
                // session.getLongCallContext().continueExec = true;
                session.getLongCallContext().getActionContext()->setRule( *rulePtr );
            }
            if ( rs.status != STATUS_OK ) break;

        }
        // else
        // smsc_log_debug(logger,"session rule for serv=%d, trans=%d not found, ok", key.serviceId, key.transport );

        // pop the key
        session.dropInitRuleKey(key.serviceId, key.transport);

        // if (session.isNew()) session.setNew( false );
        // throw RuleEngineException(0,"Cannot process Rule with ID=%d: Rule not found", 0 ); // session.getRuleKey().serviceId );

    }
    if ( rs.status == STATUS_LONG_CALL ) {
        session.getLongCallContext().getActionContext()->setRule( *rulePtr );
    }

    return;
}


void RuleEngineImpl::init( const std::string& dir )
{
    smsc_log_info(logger,"");
    smsc_log_info(logger,"Rule Engine initialization...");

    //setlocale(LC_ALL,"ru_RU.KOI8-R");
    //setlocale(LC_ALL,"UTF-8");

    //RegExp::InitLocale();

    TransportTypeHash["SMPP"] = SMPP;
    TransportTypeHash["HTTP"] = HTTP;
    TransportTypeHash["MMS"] = MMS;

    // prefetch transport rules
    // getTransportRule( SMPP );

    Property property;

    property.setBool(true);
    ConstantsHash["TRUE"] = property;

    property.setBool(false);
    ConstantsHash["FALSE"] = property;

    property.setInt(Const::USSD);
    ConstantsHash["USSD"] = property;

    property.setInt(Const::SMS);
    ConstantsHash["SMS"] = property;

    property.setInt(Const::MMS);
    ConstantsHash["MMS"] = property;

    property.setInt(Const::WAP);
    ConstantsHash["WAP"] = property;

    property.setInt(Const::HTTP);
    ConstantsHash["HTTP"] = property;

    property.setInt(sessions::OPERATION_INITED);
    ConstantsHash["ICC_INITED"] = property;

    property.setInt(sessions::OPERATION_CONTINUED);
    ConstantsHash["ICC_CONTINUED"] = property;

    property.setInt(sessions::OPERATION_COMPLETED);
    ConstantsHash["ICC_COMPLETED"] = property;

    property.setInt(dsdUnknown);
    ConstantsHash["UNKNOWN"] = property;

    property.setInt(dsdSrv2Srv);
    ConstantsHash["DIRECTION_SME_2_SME"] = property;

    property.setInt(dsdSrv2Sc);
    ConstantsHash["DIRECTION_SME_2_SC"] = property;

    property.setInt(dsdSc2Srv);
    ConstantsHash["DIRECTION_SC_2_SME"] = property;

    property.setInt(dsdSc2Sc);
    ConstantsHash["DIRECTION_SC_2_SC"] = property;

    rules = new Rules();

    try
    {
        //XMLPlatformUtils::Initialize("ru_RU.KOI8-R");
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());
        throw RuleEngineException(0,"Error during initialization XMLPlatform: %s", msg.localForm());
    }

    RulesDir = dir;


    char * transportkey = 0;
    scag::transport::TransportType value;

    std::string currentDir;

    TransportTypeHash.First();
    for (Hash <scag::transport::TransportType>::Iterator it = TransportTypeHash.getIterator(); it.Next(transportkey, value);)
    {
        currentDir = dir;
        currentDir.append("/");
        currentDir.append(transportkey);

        smsc_log_info(logger,"Rule Engine: Init transport %s...", transportkey);
        ReadRulesFromDir(value, currentDir.c_str());
        smsc_log_info(logger,"Rule Engine: Transport %s inited", transportkey);
    }

    smsc_log_info(logger,"Rule Engine inited successfully...");
    smsc_log_info(logger,"");
}


void RuleEngineImpl::updateRule(RuleKey& key)
{
    MutexGuard mg(changeLock);

    std::string filename = CreateRuleFileName(RulesDir,key);
    Rule* newRule = ParseFile(filename);
    if (!newRule) 
        throw SCAGException("Cannod load rule %d from file %s", key.serviceId, filename.c_str());

    Rules *newRules = copyReference();
    Rule** rulePtr = newRules->rules.GetPtr(key);

    if (rulePtr)
    {
        (*rulePtr)->unref();
        newRules->rules.Delete(key);
    }

    newRules->rules.Insert(key, newRule);
    changeRules(newRules);
}


void RuleEngineImpl::removeRule(RuleKey& key)
{
    MutexGuard mg(changeLock);

    Rule** rulePtr = rules->rules.GetPtr(key);  // Can we do such direct access? TODO: Ensure
    if (!rulePtr)
        throw SCAGException("Invalid rule id %d to remove", key.serviceId);

    Rules *newRules = copyReference();
    rulePtr = newRules->rules.GetPtr(key);
    (*rulePtr)->unref();
    newRules->rules.Delete(key);
    changeRules(newRules);
}

RuleEngineImpl::RuleEngineImpl() :
rules(0)
{
    logger = Logger::getInstance("scag.re");
}

RuleEngineImpl::~RuleEngineImpl()
{
    XMLPlatformUtils::Terminate();
    if (rules) rules->unref();

    smsc_log_debug(logger,"Rule Engine released");
}

}
}
