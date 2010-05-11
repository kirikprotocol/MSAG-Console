/* $Id$ */

#ifndef SCAG_BILL_BASE_INFRASTRUCTURE
#define SCAG_BILL_BASE_INFRASTRUCTURE

#include "sms/sms.h"

namespace scag2 {
namespace bill {
namespace infrastruct {

using namespace smsc::sms;

enum { // bill type
    STAT,
    NONE,
    INMAN,
    INMANSYNC,
    EWALLET
}; // bill type


class TariffRec
{
public:
    inline static uint32_t makeHashKeyChecked( uint32_t cat, uint32_t media, uint32_t oper ) {
        return ((cat % 0x1ff) * 0x1ff + (media % 0x1ff))*0xfff + (oper % 0xfff);
    }

public:
    std::string ServiceNumber;
    std::string Currency;
    uint32_t MediaTypeId;
    uint32_t CategoryId;
    uint32_t billType;

    TariffRec() : fPrice(0) {}
    TariffRec(const std::string& sn, const std::string& pr, const std::string& c,
              uint32_t ci, uint32_t mti, uint32_t bt) :
    ServiceNumber(sn), Currency(c), MediaTypeId(mti), CategoryId(ci), billType(bt), sPrice(pr), fPrice(0) {
        fPrice = atof(pr.c_str());
    }
    
    TariffRec(const TariffRec& sm) { operator=(sm); };

    TariffRec& operator=(const TariffRec& sm)
    {
        sPrice = sm.sPrice;
        fPrice = sm.fPrice;
        ServiceNumber = sm.ServiceNumber;
        Currency = sm.Currency;
        CategoryId = sm.CategoryId;
        MediaTypeId = sm.MediaTypeId;
        billType = sm.billType;
        return *this;
    }

    void setPrice( const std::string& price ) {
        sPrice = price;
        fPrice = atof(sPrice.c_str());
    }
    const std::string& getPrice() const { return sPrice; }
    double             getFloatPrice() const { return fPrice; }
    int                getIntPrice() const { return fPrice >= 0 ? int(fPrice+0.5) : int(fPrice-0.5); }

private:
    std::string sPrice;
    double      fPrice;
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

#endif // SCAG_BILL_BASE_INFRASTRUCTURE
