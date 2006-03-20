/* $Id$ */

#include <sys/types.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <locale.h>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

#include <scag/util/singleton/Singleton.h>
#include <logger/Logger.h>

#include "XMLHandlers.h"
#include "util/regexp/RegExp.hpp"

#include "Infrastructure.h"

XERCES_CPP_NAMESPACE_USE

namespace scag { namespace bill { namespace infrastruct {

using smsc::core::buffers::IntHash;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;
using namespace smsc::util;
using namespace scag::util::singleton;
using namespace smsc::util::regexp;
using smsc::logger::Logger;

class InfrastructureImpl : public Infrastructure
{
	typedef IntHash<uint32_t> ServiceHash;

	ServiceHash* service_hash;
    std::string xmlFile;
    Logger * logger;
	Mutex ProviderReloadMutex, ProviderMapMutex;
	Mutex OperatorReloadMutex, OperatorMapMutex;

    void ParseFile(const std::string& xmlFile, IntHash<uint32_t>*);

public:
    InfrastructureImpl();
    ~InfrastructureImpl();

    virtual void ReloadProviderMap();
    virtual void ReloadOperatorMap();
    virtual uint32_t GetProviderID(uint32_t service_id);
    virtual uint32_t GetOperatorID(Address addr);
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


static bool  bInfrastructureInited = false;
static Mutex initInfrastructureLock;

inline unsigned GetLongevity(InfrastructureImpl*) { return 7; } // ? Move upper ? 
typedef SingletonHolder<InfrastructureImpl> SingleSM;

Infrastructure& Infrastructure::Instance()
{
    if (!bInfrastructureInited) 
    {
        MutexGuard guard(initInfrastructureLock);
        if (!bInfrastructureInited) 
            throw Exception("Infrastructure not inited!");
    }       
    return SingleSM::Instance();
}


void Infrastructure::Init(const std::string& _xmlFile)
{
    if (!bInfrastructureInited)
    {
        MutexGuard guard(initInfrastructureLock);
        if (!bInfrastructureInited) {
            InfrastructureImpl& sm = SingleSM::Instance();
//			sm.xmlFile = _xmlFile;
//		    logger = Logger::getInstance("bill.i");
//            sm.Reload(); 
            bInfrastructureInited = true;
        }
    }
}


void InfrastructureImpl::ParseFile(const std::string& _xmlFile, ServiceHash* h)
{
    int errorCount = 0;
    int errorCode = 0;

    SAXParser parser;
    XMLBasicHandler handler("KOI8-R", h);

    setlocale(LC_ALL,"ru_RU.KOI8-R");
    //setlocale(LC_ALL,"UTF-8");
    RegExp::InitLocale();

    try
    {
        XMLPlatformUtils::Initialize("ru_RU.KOI8-R");

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

	    XMLPlatformUtils::Terminate();
    }
    catch (const OutOfMemoryException&)
    {
        smsc_log_error(logger,"Terminate parsing Rule: XMLPlatform: OutOfMemoryException");
        throw Exception("Terminate parsing Rule: XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        smsc_log_error(logger,"Terminate parsing Rule: An error occurred. Error: %s", msg.localForm());
        throw Exception("Terminate parsing Rule: An error occurred. Error: %s", msg.localForm());
    }
    catch (Exception& e)
    {
        smsc_log_error(logger,"Terminate parsing Rule: %s",e.what());
        throw e;
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing Rule: unknown fatal error");
        throw Exception("Terminate parsing Rule: unknown fatal error");
    }

    //delete parser;

    if (errorCount > 0) 
        smsc_log_error(logger,"Error parsing Rule: some errors occured");
}

InfrastructureImpl::InfrastructureImpl() : service_hash(NULL)
{
}

InfrastructureImpl::~InfrastructureImpl()
{
	if(service_hash != NULL)
		delete service_hash;
    smsc_log_debug(logger,"Service Mapper released");
}

void InfrastructureImpl::ReloadProviderMap()
{
	MutexGuard mt(ProviderReloadMutex);
	ServiceHash *h = new ServiceHash();
	try{
		ParseFile(xmlFile, h);
		MutexGuard mt1(ProviderMapMutex);
		delete service_hash;
		service_hash = h;
	}
	catch(Exception e)
	{
		smsc_log_info(logger, "Service Map reload was not successful");
		delete h;
	}
}

void InfrastructureImpl::ReloadOperatorMap()
{
	MutexGuard mt(OperatorReloadMutex);
}

uint32_t InfrastructureImpl::GetProviderID(uint32_t service_id)
{
	MutexGuard mt(ProviderMapMutex);
	try{
		return service_hash->Get(service_id);
	}
	catch(HashInvalidKeyException e)
	{
		return 0;
	}
}

uint32_t InfrastructureImpl::GetOperatorID(Address addr)
{
	return 0;
}

}}}
