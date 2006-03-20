#ifndef SCAG_BILL_INFRASTRUCTURE
#define SCAG_BILL_INFRASTRUCTURE

#include "sms/sms.h"

namespace scag { namespace bill { namespace infrastruct {

class Infrastructure
{
    Infrastructure(const Infrastructure& sm);
    Infrastructure& operator=(const Infrastructure& sm);

protected:

    Infrastructure() {};
    virtual ~Infrastructure() {};

public:

    static void Init(const std::string& filename);
    static Infrastructure& Instance();

	virtual uint32_t GetProviderID(uint32_t service_id);
	virtual uint32_t GetOperatorID(struct Address addr);
	virtual void ReloadProviderMap();
	virtual void ReloadOperatorMap();
};

}}}

#endif // SCAG_BILL_INFRASTRUCTURE
