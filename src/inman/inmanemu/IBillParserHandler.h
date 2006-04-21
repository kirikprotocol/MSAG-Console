#ifndef _INMANEMU_IBILL_PARSER_HANDLER_
#define _INMANEMU_IBILL_PARSER_HANDLER_

#include <sms/sms.h>


namespace inmanemu {


using namespace smsc::sms;



struct MatrixKey
{
    int serviceNumber;
    bool operator == (const MatrixKey& key)
    {
        return serviceNumber==key.serviceNumber;
    }
};


class IBillParserHandler
{
public:
    virtual void RegisterAccount(Address& abonentAddr, int money) = 0;
    virtual void RegisterBillRecord(MatrixKey& key, int price) = 0;
};


}

#endif
