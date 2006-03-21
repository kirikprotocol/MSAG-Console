/* $Id$ */

#include <sys/types.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <locale.h>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

#include <scag/util/singleton/Singleton.h>
#include <logger/Logger.h>
#include <sms/sms_const.h>

#include "XMLHandlers.h"

#include "Infrastructure.h"

XERCES_CPP_NAMESPACE_USE

namespace scag { namespace bill { namespace infrastruct {

using smsc::core::buffers::IntHash;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;
using namespace smsc::util;
using namespace scag::util::singleton;
using smsc::logger::Logger;
using smsc::sms;

class InfrastructureImpl : public Infrastructure
{
	typedef IntHash<uint32_t> ServiceHash;

	IntHash<uint32_t>* service_hash;
	Hash<uint32_t>*	mask_hash;
    std::string ProviderFile, OperatorFile;
    Logger * logger;
	Mutex ProviderReloadMutex, ProviderMapMutex;
	Mutex OperatorReloadMutex, OperatorMapMutex;

    void ParseFile(const char *, XMLBasicHandler*);

public:
    InfrastructureImpl();
    ~InfrastructureImpl();

    virtual void ReloadProviderMap();
    virtual void ReloadOperatorMap();
    virtual uint32_t GetProviderID(uint32_t service_id);
    virtual uint32_t GetOperatorID(Address addr);

	void _Init(const char *, const char *);
protected:
};

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


void Infrastructure::Init(const char* _ProviderFile, const char* _OperatorFile)
{
    if (!bInfrastructureInited)
    {
        MutexGuard guard(initInfrastructureLock);
        if (!bInfrastructureInited) {
            InfrastructureImpl& sm = SingleSM::Instance();
			sm._Init(_ProviderFile, _OperatorFile);
            sm.ReloadProviderMap(); 
            sm.ReloadOperatorMap(); 
            bInfrastructureInited = true;
        }
    }
}

void InfrastructureImpl::_Init(const char* _ProviderFile, const char* _OperatorFile)
{
	MutexGuard mt(ProviderReloadMutex);
	MutexGuard mt1(OperatorReloadMutex);
	ProviderFile = _ProviderFile;
	OperatorFile = _OperatorFile;
}

void InfrastructureImpl::ParseFile(const char* _xmlFile, XMLBasicHandler* handler)
{
    int errorCount = 0;
    int errorCode = 0;

//    setlocale(LC_ALL,"ru_RU.KOI8-R");
//    XMLPlatformUtils::Initialize("ru_RU.KOI8-R");

    SAXParser parser;

    //setlocale(LC_ALL,"UTF-8");
//    RegExp::InitLocale();

    try
    {
        parser.setValidationScheme(SAXParser::Val_Always);
        parser.setDoSchema(true);
        parser.setValidationSchemaFullChecking(true);
        parser.setDoNamespaces(true);

        parser.setValidateAnnotations(false);

        parser.setValidationConstraintFatal(true);

        parser.setDocumentHandler(handler);
        parser.setErrorHandler(handler);

        parser.parse(_xmlFile);
        errorCount = parser.getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
        smsc_log_error(logger,"Terminate parsing: XMLPlatform: OutOfMemoryException");
        throw Exception("Terminate parsing: XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        smsc_log_error(logger,"Terminate parsing: An error occurred. Error: %s", msg.localForm());
        throw Exception("Terminate parsing: An error occurred. Error: %s", msg.localForm());
    }
    catch (Exception& e)
    {
        smsc_log_error(logger,"Terminate parsing: %s",e.what());
        throw e;
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing: unknown fatal error");
        throw Exception("Terminate parsing: unknown fatal error");
    }

//    delete parser;
  //  XMLPlatformUtils::Terminate();

    if (errorCount > 0) 
        smsc_log_error(logger,"Error parsing: some errors occured");
}

InfrastructureImpl::InfrastructureImpl()
{
    XMLPlatformUtils::Initialize("ru_RU.KOI8-R");
    logger = Logger::getInstance("bill.i");

    smsc_log_debug(logger,"Provider/Operator Mapper allocated");

	service_hash = new IntHash<uint32_t>();
	mask_hash = new Hash<uint32_t>();
}

InfrastructureImpl::~InfrastructureImpl()
{
	if(service_hash != NULL)
		delete service_hash;
	if(mask_hash != NULL)
		delete mask_hash;

    smsc_log_debug(logger,"Provider/Operator Mapper released");

    XMLPlatformUtils::Terminate();
}

void InfrastructureImpl::ReloadProviderMap()
{
	MutexGuard mt(ProviderReloadMutex);

	smsc_log_info(logger, "ReloadProviderMap Started");

	IntHash<uint32_t> *hash = new IntHash<uint32_t>();
	try{
	    XMLBasicHandler handler(hash);
		ParseFile(ProviderFile.c_str(), &handler);

		MutexGuard mt1(ProviderMapMutex);
		delete service_hash;
		service_hash = hash;
	}
	catch(Exception& e)
	{
		smsc_log_info(logger, "Provider Map reload was not successful");
		delete hash;
	}

	smsc_log_info(logger, "ReloadProviderMap Finished");
}

void InfrastructureImpl::ReloadOperatorMap()
{
	MutexGuard mt(OperatorReloadMutex);

	smsc_log_info(logger, "ReloadOperatorMap Started");

	Hash<uint32_t> *hash = new Hash<uint32_t>();
	try{
	    XMLBasicHandler handler(hash);
		ParseFile(OperatorFile.c_str(), &handler);

		MutexGuard mt1(OperatorMapMutex);
		delete mask_hash;
		mask_hash = hash;
	}
	catch(Exception& e)
	{
		smsc_log_info(logger, "Operator Map reload was not successful");
		delete hash;
	}

	smsc_log_info(logger, "ReloadOperatorMap Finished");
}

uint32_t InfrastructureImpl::GetProviderID(uint32_t service_id)
{
	MutexGuard mt(ProviderMapMutex);
	try{
		return service_hash->Get(service_id);
	}
	catch(...)
	{
		return 0;
	}
}

uint32_t InfrastructureImpl::GetOperatorID(Address addr)
{
	MutexGuard mt(OperatorMapMutex);
	
	uint8_t mask_ptr, addr_len;
	char a[smsc::sms::MAX_ADDRESS_VALUE_LENGTH + 2];

	a[0] = '+';
	addr_len = addr.getValue(a + 1);

	if(!addr_len)
		return 0;

	addr_len++;

	mask_ptr = addr_len - 1;

	bool found;
	while(!(found = mask_hash->Exists(a)) && mask_ptr > 1)
		a[mask_ptr--] = '?';

	if(found)
		return mask_hash->Get(a);

	return 0;
}

}}}
