#include <stdio.h>
#include "interface.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "util/debug.h"

namespace smsc{
namespace smppgw{
namespace billing{

using smsc::core::buffers::IntHash;
using smsc::core::synchronization::Mutex;

class SampleBilling:IBillingInterface{
public:
  SampleBilling():trId(0)
  {
  }
  virtual TransactionIdType BeginTransaction(const smsc::sms::SMS& sms,bool serviceGenerated)
  {
    MutexGuard g(mtx);
    uint32_t newTrId=trId++;
    if(trId==InvalidTransactionId)trId++;
    TransactionData td;
    td.from=sms.getOriginatingAddress();
    td.to=sms.getDestinationAddress();
    trStore.Insert(newTrId,td);
    __trace2__("SB: begin trans %u: %s->%s",newTrId,td.from.toString().c_str(),td.to.toString().c_str());
    return newTrId;
  }
  virtual void CommitTransaction(TransactionIdType transactionId)
  {
    if(transactionId==InvalidTransactionId)return;
    MutexGuard g(mtx);
    TransactionData* td=trStore.GetPtr(transactionId);
    if(td==NULL)
    {
      __warning2__("SB: attempt to commit non-existing transaction %u",transactionId);
      return;
    }
    __trace2__("SB: commit trans %u: %s->%s",transactionId,td->from.toString().c_str(),td->to.toString().c_str());
    trStore.Delete(transactionId);
  }
  virtual void RollbackTransaction(TransactionIdType transactionId)
  {
    if(transactionId==InvalidTransactionId)return;
    MutexGuard g(mtx);
    TransactionData* td=trStore.GetPtr(transactionId);
    if(td==NULL)
    {
      __warning2__("SB: attempt to rollback non-existing transaction %u",transactionId);
      return;
    }
    __trace2__("SB: rollback trans %u: %s->%s",transactionId,td->from.toString().c_str(),td->to.toString().c_str());
    trStore.Delete(transactionId);
  }
protected:
  uint32_t trId;
  struct TransactionData{
    smsc::sms::Address from;
    smsc::sms::Address to;
  };
  IntHash<TransactionData> trStore;
  Mutex mtx;
};

}
}
}

extern "C" void* getBillingInterface()
{
  return new smsc::smppgw::billing::SampleBilling();
}
