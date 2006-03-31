#include "Session.h"
#include "scag/exc/SCAGExceptions.h"
#include <time.h>

namespace scag { namespace sessions {

using namespace scag::exceptions;
using namespace scag::re;

void Operation::detachBill()
{
    if (!m_hasBill) 
    {
        smsc_log_warn(logger,"Operation: No bill to detach");
        return;
    }

    m_hasBill = false;
}

void Operation::receiveNewPart(int currentIndex,int lastIndex)
{
    if ((currentIndex < 0)||(lastIndex < 0)||(currentIndex > lastIndex)) 
        throw SCAGException("Error: Invalid SMS index (currIndex %d, lastIndex %d)",currentIndex, lastIndex);

    if (lastIndex == 0)
    {
        m_receivedAll = true;
        return;
    }

    m_receivedParts++;
    if (lastIndex == m_receivedParts) m_receivedAll = true;
}


void Operation::receiveNewResp(int currentIndex,int lastIndex)
{
    if ((currentIndex < 0)||(lastIndex < 0)||(currentIndex > lastIndex)) 
        throw SCAGException("Error: Invalid SMS index (currIndex %d, lastIndex %d)",currentIndex, lastIndex);


    if (lastIndex == 0)
    {
        m_receivedAllResp = true;
        return;
    }

    m_receivedResp++;
    if (lastIndex == m_receivedResp) m_receivedAll = true;

}


void Operation::attachBill(int BillId)
{ 
    if (m_hasBill) throw SCAGException("Operation: Cannot attach bill - bill already attached!");

    m_hasBill = true;
    billId = BillId;

    smsc_log_debug(logger,"Operation: Bill %d attached",BillId);
}


void Operation::rollbackAll()
{
    smsc_log_debug(logger,"Operation: Rollback all");
    
    BillingManager& bm = BillingManager::Instance();

    if (m_hasBill) 
    {
        bm.rollback(billId);
        m_hasBill = false;
    }
}

//////////////////////////////////////////////Session////////////////////////////////////////////

Session::Session(const CSessionKey& key) 
    : PropertyManager(), lastAccessTime(-1), 
        bChanged(false), bDestroy(false), accessCount(0), m_pCurrentOperation(0),
        logger(0), lastOperationId(0), m_isTransact(false)
{
    logger = Logger::getInstance("scag.re");
    m_SessionKey = key;

    timeval tv;
    gettimeofday(&tv,0);

    smsc_log_debug(logger," *********** Session: msec %d", tv.tv_sec*1000 + tv.tv_usec);
    

    m_SessionPrimaryKey.abonentAddr = key.abonentAddr;
    m_SessionPrimaryKey.BornMicrotime = tv;
}

Session::~Session()
{
    // rollback all pending billing transactions & destroy session


    char * key;
    AdapterProperty * value = 0;

    PropertyHash.First();
    for (Hash <AdapterProperty *>::Iterator it = PropertyHash.getIterator(); it.Next(key, value);)
        if (value) delete value;

    ClearOperations();

    //smsc_log_debug(logger,"Session: session destroyed USR='%d' Address = '%s'",m_SessionKey.USR,m_SessionKey.abonentAddr.toString().c_str());
}


void Session::DeserializeProperty(SessionBuffer& buff)
{
    std::string name, value;

    uint32_t Count;
    buff >> Count;

    for (int i=0; i < Count; i++) 
    {
        buff >> name;
        buff >> value;

        AdapterProperty * property = new AdapterProperty(name,this,UnformatWStr(value));
        PropertyHash.Insert(name.c_str(),property);
    }
}


void Session::DeserializeOperations(SessionBuffer& buff)
{
    Operation * operation;
    int key;

    unsigned int Count;
    unsigned int BillId;
    unsigned int BillCount;

    buff >> Count;

    for (int i=0; i < Count; i++) 
    {
        operation = new Operation();

        int temp;
        buff >> temp;
        operation->m_hasBill = temp;

        if (operation->m_hasBill) buff >> operation->billId;


        buff >> operation->type;
        buff >> key;
        OperationsHash.Insert(key,operation);
    }
}


void Session::DeserializePendingOperations(SessionBuffer& buff)
{
    PendingOperation p;

    int Count;

    buff >> Count;

    for (int i=0; i<Count; i++) 
    {
        int temp;

        buff >> p.type >> p.validityTime >> temp;

        p.bStartBillingOperation = temp;
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
        std::string wstr = value->getStr();
        buff << FormatWStr(wstr).c_str();
    }
}

void Session::SerializeOperations(SessionBuffer& buff)
{
    Operation * operation;
    int key;

    COperationsHash::Iterator it = OperationsHash.First();

    buff << OperationsHash.Count();

    for (;it.Next(key, operation);)
    {              
        int temp = operation->m_hasBill;
        buff << temp;
        if (operation->m_hasBill) buff << operation->billId;

        buff << operation->type;
        buff << key;
    }
}

void Session::SerializePendingOperations(SessionBuffer& buff)
{
    std::list<PendingOperation>::iterator it;

    buff << PendingOperationList.size();

    for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
    {
        int temp = it->bStartBillingOperation;
        buff << it->type << it->validityTime << temp;
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
        buff << currentOperationId;
   
    buff << m_SessionKey.abonentAddr << (uint32_t)m_SessionKey.USR << lastAccessTime << lastOperationId << m_isTransact;
    buff << m_SmppDiscriptor.cmdType << m_SmppDiscriptor.currentIndex << m_SmppDiscriptor.lastIndex;
    buff << m_SessionPrimaryKey.abonentAddr << m_SessionPrimaryKey.BornMicrotime.tv_sec << m_SessionPrimaryKey.BornMicrotime.tv_usec;
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
        buff >> currentOperationId;
        if (OperationsHash.Exist(currentOperationId)) m_pCurrentOperation = OperationsHash.Get(currentOperationId);
    }

    buff >> m_SessionKey.abonentAddr;

    unsigned int tmp;
    buff >> tmp;
    m_SessionKey.USR = tmp;

	uint8_t c;
    buff >> lastAccessTime >> lastOperationId >> c;
	m_isTransact = c;     

    buff >> tmp;
    m_SmppDiscriptor.cmdType = (CommandOperations)tmp;

    buff >> tmp;
    m_SmppDiscriptor.currentIndex = tmp;

    buff >> tmp;
    m_SmppDiscriptor.lastIndex = tmp;

    buff >> m_SessionPrimaryKey.abonentAddr;
    buff >> m_SessionPrimaryKey.BornMicrotime.tv_sec;
    buff >> m_SessionPrimaryKey.BornMicrotime.tv_usec;

}



void Session::changed(AdapterProperty& property)
{
    AdapterProperty * oldProperty = PropertyHash.Get(property.GetName().c_str());
    oldProperty->setPureStr(property.getStr());
    bChanged = true;
}

Property* Session::getProperty(const std::string& name)
{
    AdapterProperty ** propertyPTR = PropertyHash.GetPtr(name.c_str());

    if (!propertyPTR) 
    {
        AdapterProperty * property = new AdapterProperty(name,this,"");
        PropertyHash.Insert(name.c_str(),property);
        return property;
    }
    return (*propertyPTR);
}


void Session::ClearOperations()
{
    int key;
    Operation * value;

    COperationsHash::Iterator it = OperationsHash.First();

    for (;it.Next(key, value);)
    {              
        delete value;
    }


    OperationsHash.Empty();
    PendingOperationList.clear();
    PrePendingOperationList.clear();

    m_pCurrentOperation = 0;
    lastOperationId = 0;
}

void Session::abort()
{
    int key;
    Operation * value;

    COperationsHash::Iterator it = OperationsHash.First();

    for (;it.Next(key, value);)
    {              
        value->rollbackAll();
        delete value;
    }

    OperationsHash.Empty();
    PendingOperationList.clear();
    PrePendingOperationList.clear();

    m_pCurrentOperation = 0;
    lastOperationId = 0;
    bChanged = true;

    smsc_log_debug(logger,"Session: session aborted");
}

bool Session::hasOperations() 
{
    return !(PendingOperationList.empty() && (OperationsHash.Count() == 0));
}

int Session::getNewOperationId()
{
    return ++lastOperationId;
}

bool Session::hasPending()
{
    return (PendingOperationList.size() > 0);
}

void Session::expirePendingOperation()
{
    if (PendingOperationList.size() > 0) 
    {
        smsc_log_debug(logger,"Session: pending operation has expiried");
        PendingOperationList.pop_front();
    }
}


void Session::closeCurrentOperation()
{
    smsc_log_debug(logger,"Session: close current operation (id=%d)", currentOperationId);

    delete m_pCurrentOperation;
    m_pCurrentOperation = 0;
    OperationsHash.Delete(currentOperationId);

    bChanged = true;
}


Operation * Session::setCurrentOperation(int operationId)
{
    Operation ** operationPtr = OperationsHash.GetPtr(operationId);
    if (!operationPtr) throw SCAGException("Cannot find operation (id=%d)", operationId);


    currentOperationId = operationId;
    m_pCurrentOperation = (*operationPtr);

    bChanged = true;
    return (*operationPtr);
}

void Session::setOperationFromPending(SCAGCommand& cmd, int operationType)
{
    std::list<PendingOperation>::iterator it;

    for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
    {
        if (it->type == m_SmppDiscriptor.cmdType) 
        {
            AddNewOperationToHash(cmd, operationType);
            PendingOperationList.erase(it);
            return;
        }
    }
    throw SCAGException("Session: Cannot find pending operation");
}



Operation * Session::AddNewOperationToHash(SCAGCommand& cmd, int operationType)
{
    smsc_log_debug(logger,"** Session: create new operation");

    Operation * operation = new Operation();
    operation->type = operationType;

    cmd.setOperationId(getNewOperationId());
    OperationsHash.Insert(cmd.getOperationId(),operation);
    currentOperationId = cmd.getOperationId();
    m_pCurrentOperation = operation;

    bChanged = true;
    return operation;
}


void Session::addPendingOperation(PendingOperation pendingOperation)
{
    PrePendingOperationList.push_back(pendingOperation);
}

void Session::DoAddPendingOperation(PendingOperation& pendingOperation)
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
    long now;
    time(&now);
    time_t time = now;

    if (!PendingOperationList.empty()) time = (PendingOperationList.begin())->validityTime;
    return time;
}


}}