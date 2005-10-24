#include "Session.h"
#include <scag/re/CommandBrige.h>

namespace scag { namespace sessions {

using namespace scag::re;


void Operation::detachBill(int BillId)
{
    for (std::list<int>::iterator it = BillList.begin();it!=BillList.end(); ++it)
    {
        if ((*it)==BillId) 
        {
            BillList.erase(it);
            
            smsc_log_debug(logger,"Operation: Bill %d detached",BillId);
            return;
        }
    }
}

void Operation::attachBill(int BillId)
{ 
    BillList.push_front(BillId);
    smsc_log_debug(logger,"Operation: Bill %d attached",BillId);
}


void Operation::rollbackAll()
{
    smsc_log_debug(logger,"Operation: Rollback all");
    
    BillingManager& bm = BillingManager::Instance();
    for (std::list<int>::iterator it = BillList.begin();it!=BillList.end(); ++it)
    {
        bm.rollback(*it);
    }     
    BillList.clear();
}

//////////////////////////////////////////////Session////////////////////////////////////////////

void Session::DeserializeProperty(SessionBuffer& buff)
{
    std::string name, value;

    uint32_t Count;
    buff >> Count;

    for (int i=0; i < Count; i++) 
    {
        buff >> name;
        buff >> value;

        AdapterProperty * property = new AdapterProperty(name,this,value);
        PropertyHash.Insert(name.c_str(),property);
    }
}


void Session::DeserializeOperations(SessionBuffer& buff)
{
    Operation * operation;
    COperationKey key;

    unsigned int Count;
    unsigned int BillId;
    unsigned int BillCount;

    buff >> Count;

    for (int i=0; i < Count; i++) 
    {
        operation = new Operation();

        buff >> BillCount;

        for (int j=0; j < BillCount; j++) 
        {
            buff >> BillId;
            operation->BillList.push_back(BillId);
        }

        buff >> operation->type >> operation->validityTime;
        buff >> key.destAddress >> key.key;
        OperationHash.Insert(key,operation);
    }
}


void Session::DeserializePendingOperations(SessionBuffer& buff)
{
    PendingOperation p;

    int Count;

    buff >> Count;

    for (int i=0; i<Count; i++) 
    {
        buff >> p.type >> p.validityTime;
        PendingOperationList.push_back(p);
    }

}



void Session::SerializeProperty(SessionBuffer& buff)
{
    char * key;
    AdapterProperty * value = 0;

    buff << (uint32_t)PropertyHash.GetCount();

    PropertyHash.First();
    for (Hash <AdapterProperty *>::Iterator it = PropertyHash.getIterator(); it.Next(key, value);)
    {
        buff << key;
        buff << value->getStr().c_str();
    }
}

void Session::SerializeOperations(SessionBuffer& buff)
{
    Operation * operation;
    COperationKey key;

    OperationHash.First();
    XHash <COperationKey,Operation *,XOperationHashFunc>::Iterator it = OperationHash.getIterator();

    buff << OperationHash.Count();

    for (;it.Next(key, operation);)
    {              
        buff << operation->BillList.size();

        for (std::list<int>::iterator billIt = operation->BillList.begin();billIt!=operation->BillList.end(); ++billIt)
        {
            buff << (*billIt);
        }

        buff << operation->type << operation->validityTime;
        buff << key.destAddress << key.key;
    }
}

void Session::SerializePendingOperations(SessionBuffer& buff)
{
    std::list<PendingOperation>::iterator it;

    buff << PendingOperationList.size();

    for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
    {
        buff << it->type << it->validityTime;
    }
}

void Session::Serialize(SessionBuffer& buff)
{
    /*
    TODO: Check this properties:
        bool                    bChanged, bDestroy;
        int                     accessCount;
    */

    SerializeProperty(buff);
    SerializeOperations(buff);
    SerializePendingOperations(buff);

    int hasCurrentOperation = (m_pCurrentOperation != 0);
    buff << hasCurrentOperation;


    if (m_pCurrentOperation != 0) 
        buff << currentOperationKey.destAddress << currentOperationKey.key;
   
    buff << needReleaseCurrentOperation;
    buff << m_SessionKey.abonentAddr << (uint32_t)m_SessionKey.USR << lastAccessTime;
}


void Session::Deserialize(SessionBuffer& buff)
{
    buff.SetPos(0);

    DeserializeProperty(buff);
    DeserializeOperations(buff);
    DeserializePendingOperations(buff);

    int hasCurrentOperation = 0;
    buff >> hasCurrentOperation;


    m_pCurrentOperation = 0;

    if (hasCurrentOperation) 
    {
        buff >> currentOperationKey.destAddress >> currentOperationKey.key;
        if (OperationHash.Exists(currentOperationKey)) m_pCurrentOperation = OperationHash.Get(currentOperationKey);
    }

    buff >> needReleaseCurrentOperation;
    buff >> m_SessionKey.abonentAddr;

    unsigned int tmp;
    buff >> tmp;
    m_SessionKey.USR = tmp;

    buff >> lastAccessTime;     
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
        bChanged(false), bDestroy(false), accessCount(0), Owner(0),m_pCurrentOperation(0),
        needReleaseCurrentOperation(false), logger(0)
{
    logger = Logger::getInstance("scag.re");
    m_SessionKey = key;
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

    m_pCurrentOperation = 0;
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

    COperationKey operationKey;

    operationKey.destAddress = CommandBrige::getDestAddr(cmd);
    operationKey.key = CommandBrige::getKey(cmd);

    needReleaseCurrentOperation = CommandBrige::isFinalCommand(cmd);

    if (!OperationHash.Exists(operationKey)) 
    {
        Operation * operation = new Operation();
        
        //TODO: fill operation params
        OperationHash.Insert(operationKey,operation);
    }

    m_pCurrentOperation = OperationHash.Get(operationKey);
    smsc_log_error(logger,"** Session: operation started");
   
    Owner->startTimer(this->getSessionKey(), this->getWakeUpTime());
    return true;
}

void Session::releaseOperation()
{
    
    if (m_pCurrentOperation)
    {
        if (needReleaseCurrentOperation) 
        {
            OperationHash.Delete(currentOperationKey);
            m_pCurrentOperation = 0;

            smsc_log_error(logger,"** Session: operation released");
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
    return m_pCurrentOperation;
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