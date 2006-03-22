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
	virtual void ReloadProviderMap() = 0;
	virtual void ReloadOperatorMap() = 0;
	virtual void ReloadTariffMatrix() = 0;
};

class InfrastructureImpl : public Infrastructure
{
	IntHash<uint32_t>* service_hash;
	Hash<uint32_t>*	mask_hash;
    std::string ProviderFile, OperatorFile;
    smsc::logger::Logger * logger;
	Mutex ProviderReloadMutex, ProviderMapMutex;
	Mutex OperatorReloadMutex, OperatorMapMutex;
	Mutex TariffMatrixReloadMutex, TariffMatrixMapMutex;

    void ParseFile(const char *, XMLBasicHandler*);
	void SetFileNames(const char *, const char *);

public:
    InfrastructureImpl();
    ~InfrastructureImpl();

	void init(const char *, const char *);

    virtual void ReloadProviderMap();
    virtual void ReloadOperatorMap();
    virtual void ReloadTariffMatrix();
    virtual uint32_t GetProviderID(uint32_t service_id);
    virtual uint32_t GetOperatorID(Address addr);
};

}}}

#endif // SCAG_BILL_INFRASTRUCTURE
