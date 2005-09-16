#include "Session.h"

#include "scag/SAX2Print.hpp"


namespace scag { namespace sessions {

void Operation::detachBill(const Bill& bill)
{
    for (std::list<Bill>::iterator it = BillList.begin();it!=BillList.end(); ++it)
    {
        if ((*it)==bill) 
        {
            BillList.erase(it);
            
            smsc_log_debug(logger,"Operation: Bill %d detached",bill.bill_id);
            return;
        }
    }
}

void Operation::attachBill(const Bill& bill)
{ 
    BillList.push_front(bill);             
    smsc_log_debug(logger,"Operation: Bill %d attached",bill.bill_id);
}


void Operation::rollbackAll()
{
    BillingManager& bm = BillingManager::Instance();
    for (std::list<Bill>::iterator it = BillList.begin();it!=BillList.end(); ++it)
    {
        BillList.erase(it);
        bm.rollback(*it);
    }
}




void Session::changed(AdapterProperty& property)
{
    AdapterProperty * oldProperty = PropertyHash.Get(property.GetName().c_str());
    oldProperty->setPureStr(property.getStr());
}

Property* Session::getProperty(const std::string& name)
{
    AdapterProperty * property = 0;
    if (!PropertyHash.Exists(name.c_str()))
    {
        AdapterProperty * property = new AdapterProperty(name,this,"");
        PropertyHash.Insert(name.c_str(),property);
        
    }
    return PropertyHash.Get(name.c_str());
}

Session::Session(const CSessionKey& key) 
    : PropertyManager(), lastAccessTime(-1), 
        bChanged(false), bDestroy(false), accessCount(0), Owner(0),currentOperation(0),needReleaseCurrentOperation(false)
{
    m_SessionKey = key;

    //!!! Временная херота, для того, чтобы SessionManager выдавал валидную сессию
    PendingOperation operation;
    time_t now;

    time(&now);

    operation.type = 1;
    operation.validityTime = now + 100;

    addPendingOperation(operation);
    //!!! Временная херота, для того, чтобы SessionManager выдавал валидную сессию
}

Session::~Session()
{
    // rollback all pending billing transactions & destroy session


    char * key;
    AdapterProperty * value = 0;

    PropertyHash.First();
    for (Hash <AdapterProperty *>::Iterator it = PropertyHash.getIterator(); it.Next(key, value);)
        if (value) delete value;


    abort();
}


void Session::abort()
{
    if (!Owner) return;

    COperationKey key;
    Operation * value;
    
    OperationHash.First();
    XHash <COperationKey,Operation *,XOperationHashFunc>::Iterator it = OperationHash.getIterator();

    for (;it.Next(key, value);)
    {
        delete value;
    }

    OperationHash.Empty();
    PendingOperationList.clear();

    Owner->startTimer(m_SessionKey,0);
    smsc_log_error(logger,"Session: session aborted");
}

bool Session::hasOperations() 
{
    return !(PendingOperationList.empty() && (OperationHash.Count() == 0));
}

void Session::expireOperation(time_t currentTime)
{

}

bool Session::startOperation(SCAGCommand& cmd)
{
    if (!Owner) return false;

    
    //TODO: add Operation
    //Creating OperationKey

    //set needReleaseCurrentOperation flag
    
    Owner->startTimer(this->getSessionKey(), this->getWakeUpTime());
    return true;
}

void Session::releaseOperation()
{
    if (currentOperation)
    {
        if (needReleaseCurrentOperation) 
        {
            OperationHash.Delete(currentOperationKey);
            currentOperation = 0;
            Owner->startTimer(m_SessionKey,this->getWakeUpTime());
        }
    }
}

void Session::addPendingOperation(PendingOperation pendingOperation)
{

    std::list<PendingOperation>::iterator it;

    for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
    {
        if (it->validityTime > pendingOperation.validityTime) 
        {
            PendingOperationList.insert(it,pendingOperation);
            return;
        }
    }
    PendingOperationList.insert(PendingOperationList.end(),pendingOperation);
}

Operation * Session::GetCurrentOperation() const
{
    return currentOperation;
}

time_t Session::getWakeUpTime()
{
    time_t time1 = 0;
    time_t time2 = 0;

    Operation * operation = 0;
    if (!PendingOperationList.empty()) time1 = (PendingOperationList.begin())->validityTime;

    if (OperationHash.Count()) 
    {

        COperationKey key;
        Operation * value;
        time_t minTime;

        OperationHash.First();
        XHash <COperationKey,Operation *,XOperationHashFunc>::Iterator it = OperationHash.getIterator();

        OperationHash.Next(key,value);
        time2 = value->validityTime;

        for (;it.Next(key, value);)
        {
            if (minTime > value->validityTime) 
            {
                time2 = value->validityTime;
            }
        }
        
    }

    if (time1 == 0) return time2;
    if (time2 == 0) return time1;

    if (time1 > time2) return time2;
    else return time1;
}


}}