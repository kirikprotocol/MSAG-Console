/* $Id$ */

#include <sys/types.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <locale.h>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

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
using smsc::logger::Logger;
using smsc::sms;

InfrastructureImpl::InfrastructureImpl()
{
    service_hash = NULL;
    mask_hash = NULL;
    category_hash = NULL;
    media_type_hash = NULL;
    tariff_hash = NULL;
}

InfrastructureImpl::~InfrastructureImpl()
{
    if(service_hash != NULL)
        delete service_hash;
    if(mask_hash != NULL)
        delete mask_hash;
    if(category_hash != NULL)
        delete category_hash;
    if(media_type_hash != NULL)
        delete media_type_hash;
    if(tariff_hash != NULL)
        delete tariff_hash;

    smsc_log_debug(logger,"Provider/Operator Mapper released");

    XMLPlatformUtils::Terminate();
}

void InfrastructureImpl::init(const std::string& dir)
{
    XMLPlatformUtils::Initialize("ru_RU.KOI8-R");
    logger = Logger::getInstance("bill.i");

    smsc_log_debug(logger,"Provider/Operator Mapper allocated");

    service_hash = new IntHash<uint32_t>();
    mask_hash = new Hash<uint32_t>();
    category_hash = new Hash<uint32_t>();
    media_type_hash = new Hash<uint32_t>();
    tariff_hash = new IntHash<TariffRec>();


    SetFileNames(dir);

    ReloadProviderMap();
    ReloadOperatorMap();
    ReloadTariffMatrix();
}

void InfrastructureImpl::SetFileNames(const std::string& dir)
{
    MutexGuard mt(ProviderReloadMutex);
    MutexGuard mt1(OperatorReloadMutex);
    
    ProviderFile = dir + "/services.xml";
    OperatorFile = dir + "/operators.xml";
    TariffMatrixFile = dir + "/tariffs.xml";
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
        if(service_hash != NULL)
            delete service_hash;
        service_hash = hash;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Provider Map reload was not successful");
        delete hash;
        throw e;
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
        if(mask_hash != NULL)
            delete mask_hash;
        mask_hash = hash;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Operator Map reload was not successful");
        delete hash;
        throw e;
    }

    smsc_log_info(logger, "ReloadOperatorMap Finished");
}

void InfrastructureImpl::ReloadTariffMatrix()
{
    MutexGuard mt(TariffMatrixReloadMutex);

    smsc_log_info(logger, "ReloadTariffMatrix Started");

    Hash<uint32_t> *cat_hash = new Hash<uint32_t>();
    Hash<uint32_t> *mt_hash = new Hash<uint32_t>();
    IntHash<TariffRec> *t_hash = new IntHash<TariffRec>();

    try{
        XMLTariffMatrixHandler handler(cat_hash, mt_hash, t_hash);
        ParseFile(TariffMatrixFile.c_str(), &handler);

        MutexGuard mt1(TariffMatrixMapMutex);
        if(category_hash != NULL)
            delete category_hash;
        if(media_type_hash != NULL)
            delete media_type_hash;
        if(tariff_hash != NULL)
            delete tariff_hash;
        category_hash = cat_hash;
        media_type_hash = mt_hash;
        tariff_hash = t_hash;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Tariff Matrix reload was not successful");
        delete cat_hash;
        delete mt_hash;
        delete t_hash;
        throw e;
    }

    smsc_log_info(logger, "ReloadTariffMatrix Finished");
}

uint32_t InfrastructureImpl::GetProviderID(uint32_t service_id)
{
    MutexGuard mt(ProviderMapMutex);
    try{
        if(service_hash == NULL)
            return 0;

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
    
    if(mask_hash == NULL)
        return 0;

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

TariffRec* InfrastructureImpl::GetTariff(uint32_t operator_id, const char* category, const char* mt)
{
    MutexGuard mt1(TariffMatrixMapMutex);
    TariffRec *tr = NULL;
    try{
        if(category_hash == NULL || media_type_hash == NULL || tariff_hash == NULL)
            return 0;

        uint32_t id = (category_hash->Get(category) & 0x1ff) << 23;
           id |= (media_type_hash->Get(mt) & 0x1FF) << 14;
           id |= operator_id & 0xFFF;

        tr = new TariffRec();
        *tr = tariff_hash->Get(id);

        return tr;
    }
    catch(...)
    {
        if(tr != NULL)
            delete tr;
        return NULL;
    }
}

void InfrastructureImpl::ParseFile(const char* _xmlFile, HandlerBase* handler)
{
    int errorCount = 0;

    SAXParser parser;
    
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
        throw Exception("XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        smsc_log_error(logger,"Terminate parsing: XMLException: %s", msg.localForm());
        throw Exception("XMLException: %s", msg.localForm());
    }
    catch (Exception& e)
    {
        smsc_log_error(logger,"Terminate parsing: %s",e.what());
        throw e;
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing: unknown fatal error");
        throw Exception("unknown fatal error");
    }

    if (errorCount > 0) 
        smsc_log_error(logger,"Error parsing: some errors occured");
}

}}}
