#ifndef _INMANEMU_BILL_PROCESSOR_
#define _INMANEMU_BILL_PROCESSOR_

#include <logger/Logger.h>
#include <core/buffers/XHash.hpp>
#include <core/buffers/IntHash.hpp>

#include <scag/util/sms/HashUtil.h>

#include "IBillParserHandler.h"


namespace inmanemu { namespace processor {

using namespace smsc::sms;
using smsc::logger::Logger;
using namespace smsc::core::buffers;


struct HashFunc {
  static unsigned int CalcHash(const MatrixKey& key)
  {
      return key.serviceNumber;
  }
};

struct Account
{
    int amount;
    int charged;
    Account(int money) : charged(0), amount(money) {};
    Account() : charged(0), amount(0) {};
};

typedef XHash<Address,Account,scag::util::sms::XAddrHashFunc> CAccountsHash;
typedef XHash<MatrixKey, int, HashFunc> CBillRecordsHash;
typedef IntHash<Address> CBillIdDataHash; 

class BillProcessor : IBillParserHandler
{
    Logger * logger;

    CAccountsHash AccountsHash;
    CBillRecordsHash BillRecordsHash;
    CBillIdDataHash BillIdDataHash;
public:
   void init();
   void commit(int BillId);
   void rollback(int BillId);

   bool charge(MatrixKey& key, Address& abonent, int BillId);

   virtual void RegisterAccount(Address& abonentAddr, int money);
   virtual void RegisterBillRecord(MatrixKey& key, int price);

   ~BillProcessor();
};


}}

#endif

