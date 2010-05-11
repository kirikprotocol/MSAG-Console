/* $Id$ */

#include <sys/types.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <locale.h>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

#include <logger/Logger.h>

#include "scag/util/singleton/XercesSingleton.h"
#include "XMLHandlers.h"
#include "Infrastructure.h"
#include "scag/util/HRTimer.h"

namespace scag2 {
namespace bill {
namespace infrastruct {

XERCES_CPP_NAMESPACE_USE

using smsc::core::buffers::IntHash;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;
using namespace smsc::util;
using smsc::logger::Logger;
using namespace smsc::sms;

InfrastructureImpl::InfrastructureImpl()
{
    service_hash = NULL;
    mask_hash = NULL;
    category_hash = NULL;
    media_type_hash = NULL;
    category_str_hash = NULL;
    media_type_str_hash = NULL;
    tariff_hash = NULL;
}

InfrastructureImpl::~InfrastructureImpl()
{
    delete service_hash;
    delete mask_hash;
    delete category_hash;
    delete media_type_hash;
    delete tariff_hash;

    smsc_log_debug(logger,"Provider/Operator Mapper released");

    XMLPlatformUtils::Terminate();
}

void InfrastructureImpl::init(const std::string& dir)
{
    // XMLPlatformUtils::Initialize("en_EN.UTF-8");
    util::singleton::XercesSingleton::Instance();

    logger = Logger::getInstance("bill.inf");

    smsc_log_debug(logger,"Provider/Operator Mapper allocated");

    service_hash = new IntHash<uint32_t>();
    mask_hash = new Hash<uint32_t>();
    category_hash = new IntHash<uint32_t>();
    media_type_hash = new IntHash<uint32_t>();
    tariff_hash = new IntHash<TariffRec>();


    SetFileNames(dir);

    ReloadProviderMap();
    ReloadOperatorMap();

    try{
        ReloadTariffMatrix();
    }
    catch(Exception& e)
    {
    }
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
        delete service_hash;
        service_hash = hash;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Provider Map reload was not successful");
        delete hash;
        throw;
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
        throw;
    }

    smsc_log_info(logger, "ReloadOperatorMap Finished");
}

void InfrastructureImpl::ReloadTariffMatrix()
{
    MutexGuard mt(TariffMatrixReloadMutex);

    std::string res;
    res.reserve(200);
    util::HRTiming hr(res);
    hr.reset(res);

    smsc_log_info(logger, "ReloadTariffMatrix Started");

    IntHash<uint32_t> *cat_hash = new IntHash<uint32_t>();
    IntHash<uint32_t> *mt_hash = new IntHash<uint32_t>();
    Hash<uint32_t> *cat_str_hash = new Hash<uint32_t>();
    Hash<uint32_t> *mt_str_hash = new Hash<uint32_t>();
    IntHash<TariffRec> *t_hash = new IntHash<TariffRec>();

    try{
        XMLTariffMatrixHandler handler(cat_hash, mt_hash, cat_str_hash, mt_str_hash, t_hash);
        ParseFile(TariffMatrixFile.c_str(), &handler);
        hr.mark("parsed");
        {
            MutexGuard mt1(TariffMatrixMapMutex);
            hr.mark("locked");
            std::swap(category_hash,cat_hash);
            std::swap(media_type_hash,mt_hash);
            std::swap(category_str_hash,cat_str_hash);
            std::swap(media_type_str_hash,mt_str_hash);
            std::swap(tariff_hash,t_hash);
        }
        delete cat_hash;
        delete mt_hash;
        delete cat_str_hash;
        delete mt_str_hash;
        delete t_hash;
        hr.mark("dtor");
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Tariff Matrix reload was not successful");
        delete cat_hash;
        delete mt_hash;
        delete cat_str_hash;
        delete mt_str_hash;
        delete t_hash;
        throw;
    }
    hr.mark("done");
    smsc_log_info(logger, "ReloadTariffMatrix Finished, %s, cat=%u/%u, med=%u/%u, catstr=%u/%u/%u medstr=%u/%u/%u, tar=%u/%u",
                  res.c_str(),
                  category_hash->Count(),
                  category_hash->Size(),
                  media_type_hash->Count(),
                  media_type_hash->Size(),
                  category_str_hash->GetCount(),
                  category_str_hash->GetBucketsCount(),
                  category_str_hash->GetUsage(),
                  media_type_str_hash->GetCount(),
                  media_type_str_hash->GetBucketsCount(),
                  media_type_str_hash->GetUsage(),
                  tariff_hash->Count(),
                  tariff_hash->Size() );
}

uint32_t InfrastructureImpl::GetProviderID(uint32_t service_id)
{
    MutexGuard mt(ProviderMapMutex);
    if(service_hash == NULL) return 0;
    
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
    uint32_t ret =0;
    do {
        if (mask_hash == NULL) break;

        std::string a = addr.toString();
        uint8_t mask_ptr = a.length();

        bool found;
        {
            MutexGuard mt(OperatorMapMutex);
            if (mask_hash == NULL) break;
            while(!(found = mask_hash->Exists(a.c_str())) && mask_ptr > 5)
            {
                //        smsc_log_debug(logger, "Trying mask: %s", a.c_str());
                a[--mask_ptr] = '?';
            }
            if (found) ret = mask_hash->Get(a.c_str());
        }

        if ( logger->isDebugEnabled() ) {
            if (found) {
                smsc_log_debug(logger, "Match mask: %s, operator=%u", a.c_str(), ret);
            } else {
                smsc_log_debug(logger, "No match mask found for: %s", addr.toString().c_str());
            }
        }
    } while ( false );
    return ret;
}

TariffRec* InfrastructureImpl::GetTariff(uint32_t operator_id, uint32_t category, uint32_t mt)
{
    MutexGuard mt1(TariffMatrixMapMutex);
    TariffRec *tr = NULL;
    try{
        if(category_hash == NULL || media_type_hash == NULL || tariff_hash == NULL)
            return NULL;

        /*
        uint32_t id = (category_hash->Get(category) & 0x1ff) << 23;
           id |= (media_type_hash->Get(mt) & 0x1FF) << 14;
           id |= operator_id & 0xFFF;
         */

        const uint32_t id = 
            TariffRec::makeHashKeyChecked(category_hash->Get(category),
                                          media_type_hash->Get(mt),
                                          operator_id);
        tr = new TariffRec(tariff_hash->Get(id));

        return tr;
    }
    catch(...)
    {
        delete tr;
        return NULL;
    }
}

uint32_t InfrastructureImpl::GetMediaTypeID(const std::string& media_type_str)
{
    MutexGuard mt(TariffMatrixMapMutex);
    if(media_type_str_hash == NULL)
        return 0;
    uint32_t *p = media_type_str_hash->GetPtr(media_type_str.c_str());
    return p ? *p : 0;
}

uint32_t InfrastructureImpl::GetCategoryID(const std::string& category_str)
{
    MutexGuard mt(TariffMatrixMapMutex);
    if(category_str_hash == NULL)
        return 0;
    uint32_t *p = category_str_hash->GetPtr(category_str.c_str());
    return p ? *p : 0;
}

bool InfrastructureImpl::GetTariff(uint32_t operator_id, uint32_t category, uint32_t mt, TariffRec& tr)
{
    MutexGuard mt1(TariffMatrixMapMutex);
    
    if(category_hash == NULL || media_type_hash == NULL || tariff_hash == NULL) return false;
    
    try{

        const uint32_t id = 
            TariffRec::makeHashKeyChecked(category_hash->Get(category),
                                          media_type_hash->Get(mt),
                                          operator_id);
        tr = tariff_hash->Get(id);
        return true;
    }
    catch(...)
    {
        return false;
    }
}

void InfrastructureImpl::ParseFile(const char* _xmlFile, HandlerBase* handler)
{
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
        throw;
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing: unknown fatal error");
        throw Exception("unknown fatal error");
    }
}

}}}
