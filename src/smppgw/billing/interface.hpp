#ifndef __SMSC_SMPPGW_BILLING_INTERFACE__
#define __SMSC_SMPPGW_BILLING_INTERFACE__

#include "util/int.h"
#include  <string>
#include "sms/sms.h"

namespace smsc{
namespace smppgw{
namespace billing{

typedef uint32_t TransactionIdType;

static const uint32_t InvalidTransactionId=0xFFFFFFFFU;

class IBillingInterface{
public:
  virtual TransactionIdType BeginTransaction(const smsc::sms::SMS& sms,bool serviceGenerated)=0;
  virtual void CommitTransaction(TransactionIdType transactionId)=0;
  virtual void RollbackTransaction(TransactionIdType transactionId)=0;
};

}//billing
}//smppgw
}//smsc

#endif
