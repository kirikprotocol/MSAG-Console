/* $Id$ */

#ifndef SCAG_BILL_INFRASTRUCTURE
#define SCAG_BILL_INFRASTRUCTURE

#include "sms/sms.h"
#include "core/buffers/IntHash.hpp"
#include "XMLHandlers.h"

namespace scag { namespace bill { namespace infrastruct {

using namespace smsc::sms;

class Infrastructure
{
protected:
    Infrastructure(const Infrastructure& sm);
    Infrastructure& operator=(const Infrastructure& sm);

    Infrastructure() {};
    virtual ~Infrastructure() {};

public:

    virtual uint32_t GetProviderID(uint32_t service_id) = 0;
    virtual uint32_t GetOperatorID(Address addr) = 0;
    virtual TariffRec* GetTariff(uint32_t operator_id, uint32_t category, uint32_t mt) = 0;
    virtual void ReloadProviderMap() = 0;
    virtual void ReloadOperatorMap() = 0;
    virtual void ReloadTariffMatrix() = 0;
};

class InfrastructureImpl : public Infrastructure
{
    IntHash<uint32_t>* service_hash;
    Hash<uint32_t>* mask_hash;
    IntHash<uint32_t>* category_hash;
    IntHash<uint32_t>* media_type_hash;
    IntHash<TariffRec>* tariff_hash;
    std::string ProviderFile, OperatorFile, TariffMatrixFile;
    smsc::logger::Logger * logger;
    Mutex ProviderReloadMutex, ProviderMapMutex;
    Mutex OperatorReloadMutex, OperatorMapMutex;
    Mutex TariffMatrixReloadMutex, TariffMatrixMapMutex;

    void ParseFile(const char *, HandlerBase*);
    void SetFileNames(const std::string&);

public:
    InfrastructureImpl();
    ~InfrastructureImpl();

    void init(const std::string&);

    virtual void ReloadProviderMap();
    virtual void ReloadOperatorMap();
    virtual void ReloadTariffMatrix();
    virtual uint32_t GetProviderID(uint32_t service_id);
    virtual uint32_t GetOperatorID(Address addr);
    virtual TariffRec* GetTariff(uint32_t operator_id, uint32_t category, uint32_t mt);
};

}}}

#endif // SCAG_BILL_INFRASTRUCTURE
