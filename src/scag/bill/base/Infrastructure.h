/* $Id$ */

#ifndef SCAG_BILL_INFRASTRUCTURE_BASE
#define SCAG_BILL_INFRASTRUCTURE_BASE

#include "sms/sms.h"

namespace scag2 {
namespace bill {
namespace infrastruct {

using namespace smsc::sms;

enum {
    STAT,
    NONE,
    INMAN,
    INMANSYNC
};


class TariffRec
{
public:
    double Price;
    std::string ServiceNumber;
    std::string Currency;
    uint32_t MediaTypeId;
    uint32_t CategoryId;
    uint32_t billType;

    TariffRec() {};
    TariffRec(const std::string& sn, double pr, const std::string& c, uint32_t ci, uint32_t mti, uint32_t bt):Price(pr), ServiceNumber(sn), Currency(c), MediaTypeId(mti), CategoryId(ci), billType(bt) {};
    
    TariffRec(const TariffRec& sm) { operator=(sm); };

    TariffRec& operator=(const TariffRec& sm)
    {
        Price = sm.Price;
        ServiceNumber = sm.ServiceNumber;
        Currency = sm.Currency;
        CategoryId = sm.CategoryId;
        MediaTypeId = sm.MediaTypeId;
        billType = sm.billType;

        return *this;
    };
};


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
    virtual uint32_t GetMediaTypeID(const std::string& media_type_str) = 0;
    virtual uint32_t GetCategoryID(const std::string& category_str) = 0;
    virtual TariffRec* GetTariff(uint32_t operator_id, uint32_t category, uint32_t mt) = 0;
    virtual bool GetTariff(uint32_t operator_id, uint32_t category, uint32_t mt, TariffRec& tr) = 0;
    virtual void ReloadProviderMap() = 0;
    virtual void ReloadOperatorMap() = 0;
    virtual void ReloadTariffMatrix() = 0;
};

}}}

#endif // SCAG_BILL_INFRASTRUCTURE
