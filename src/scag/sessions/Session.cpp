#include "Session.h"
#include "scag/bill/BillingManager.h"
#include "scag/re/CommandBrige.h"
#include "scag/exc/SCAGExceptions.h"
#include <time.h>

namespace scag { namespace sessions {

using namespace scag::exceptions;
using namespace scag::re;

////////////////////////////////////////////////STATIC FUNCTIONS//////////////////////////

smsc::core::synchronization::Mutex Operation::loggerMutex;
smsc::logger::Logger* Operation::logger = NULL;

smsc::core::synchronization::Mutex Session::loggerMutex;
smsc::logger::Logger* Session::logger = NULL;

smsc::core::synchronization::Mutex Session::cntMutex;
uint32_t Session::sessionCounter = 0;
uint32_t Session::stuid = 0;

Hash<int> Session::InitOperationTypesHash()
{
    Hash<int> hs;

    hs["DELIVER"] = CO_DELIVER;
    hs["SUBMIT"] = CO_SUBMIT;
    hs["RECEIPT"] = CO_RECEIPT;

    hs["USSD_DIALOG"] = CO_USSD_DIALOG;

    hs["HTTP_DELIVERY"] = CO_HTTP_DELIVERY;

    hs["DATA_SC_2_SME"] = CO_DATA_SC_2_SME;
    hs["DATA_SC_2_SC"] = CO_DATA_SC_2_SC;
    hs["DATA_SME_2_SME"] = CO_DATA_SME_2_SME;
    hs["DATA_SME_2_SC"] = CO_DATA_SME_2_SC;

    return hs;
}


Hash<int> Session::OperationTypesHash = Session::InitOperationTypesHash();

Hash<int> Session::InitReadOnlyPropertiesHash()
{
    Hash<int> hs;

    hs["USR"]               = PROPERTY_USR;
    hs["ICC_STATUS"]        = PROPERTY_ICC_STATUS;

    return hs;
}

Hash<int> Session::ReadOnlyPropertiesHash = Session::InitReadOnlyPropertiesHash();

////////////////////////////////////////////////STATIC FUNCTIONS//////////////////////////
////////////////////////////////////////////////STATIC FUNCTIONS//////////////////////////

bool Session::isReadOnlyProperty(const char * name)
{
    return (ReadOnlyPropertiesHash.GetPtr(name));
}


int Session::getOperationType(std::string& str)
{
    int * pType = OperationTypesHash.GetPtr(str.c_str());
    if (!pType) throw SCAGException("Unknown OperationType '%s'", str.c_str());

    return (*pType);
}


void PendingOperation::rollbackAll(bool timeout)
{
    if(billID <= 0) return;
    smsc_log_warn(logger, "PendingOperation: Rollback all (billId=%d)", billID);
    try
    {
        scag::bill::BillingManager::Instance().Rollback(billID, timeout);
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"PendingOperation: Cannot rollback. Details: %s", e.what());
    }
    billID = 0;
}


ICCOperationStatus Operation::getStatus()
{
    return m_Status;
}

void Operation::detachBill()
{
    if (!m_hasBill)
    {
        smsc_log_warn(logger,"Operation: (ab=%s) No bill to detach", m_Owner->m_SessionKey.abonentAddr.toString().c_str());
        return;
    }

    smsc_log_debug(logger,"Operation: Bill (id=%d, ab=%s) dettached", billId, m_Owner->m_SessionKey.abonentAddr.toString().c_str());
  billId = 0;
    m_hasBill = false;
}

void Operation::receiveNewPart(int currentIndex,int lastIndex)
{
    //smsc_log_debug(logger,"Operation: Change status part (CI=%d, LI=%d, allParts=%d)",currentIndex, lastIndex, m_receivedAllParts);

    if ((currentIndex < 0)||(lastIndex < 0)||(currentIndex > lastIndex))
        throw SCAGException("Error: Invalid SMS index (currIndex %d, lastIndex %d)",currentIndex, lastIndex);

    if (currentIndex == 0)
        m_Status = OPERATION_INITED;
    else
        m_Status = OPERATION_CONTINUED;


    if (lastIndex == 0)
    {
        m_receivedAllParts = true;
        if ((m_receivedAllResp)&&(m_receivedAllParts)) m_Status = OPERATION_COMPLETED;

        return;
    }

    m_receivedParts++;
    m_receivedAllParts = (lastIndex <= m_receivedParts);

    if ((m_receivedAllResp)&&(m_receivedAllParts)) m_Status = OPERATION_COMPLETED;
}


void Operation::receiveNewResp(int currentIndex,int lastIndex)
{
    //smsc_log_debug(logger,"Operation: Change status resp (CI=%d, LI=%d, allParts=%d)",currentIndex, lastIndex, m_receivedAllParts);


    if ((currentIndex < 0)||(lastIndex < 0)||(currentIndex > lastIndex))
        throw SCAGException("Error: Invalid SMS index (currIndex %d, lastIndex %d)",currentIndex, lastIndex);

    m_Status = OPERATION_CONTINUED;

    if (lastIndex == 0)
    {
        m_receivedAllResp = true;
        if ((m_receivedAllResp)&&(m_receivedAllParts)) m_Status = OPERATION_COMPLETED;

        return;
    }

    m_receivedResp++;
    m_receivedAllResp = (lastIndex <= m_receivedResp);

    if ((m_receivedAllResp)&&(m_receivedAllParts)) m_Status = OPERATION_COMPLETED;

}


void Operation::attachBill(unsigned int BillId)
{
    if (m_hasBill) throw SCAGException("Operation: (ab=%s) Cannot attach bill - bill already attached!", m_Owner->m_SessionKey.abonentAddr.toString().c_str());

    m_hasBill = true;
    billId = BillId;

    smsc_log_debug(logger,"Operation: Bill (id=%d) attached (ab=%s)",BillId, m_Owner->m_SessionKey.abonentAddr.toString().c_str());
}


void Operation::rollbackAll()
{
    if(!m_hasBill) return;
    smsc_log_warn(logger,"Operation: Rollback all (ab=%s)", m_Owner->m_SessionKey.abonentAddr.toString().c_str());
    try
    {
        scag::bill::BillingManager::Instance().Rollback(billId);
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"Operation: Cannot rollback. Details: %s", e.what());
    }
    m_hasBill = false;
}

//////////////////////////////////////////////Session////////////////////////////////////////////

Session::Session(const CSessionKey& key)
    : PropertyManager(), lastAccessTime(-1),
        bChanged(false), bDestroy(false), accessCount(0), m_pCurrentOperation(0),
        lastOperationId(0), m_CanOpenSubmitOperation(false), m_bRedirectFlag(false),
        m_SessionPrimaryKey(key.abonentAddr), bIsNew(true), bIsExpired(false), deleteScheduled(false),offset(0)
{
    if(!logger)
    {
        MutexGuard mt(loggerMutex);
        if(!logger) logger = Logger::getInstance("sess.man");
    }

    uint32_t sc = 0;
    {
        MutexGuard mtxx(cntMutex);
        sc = ++sessionCounter;
        uid = ++stuid;
    }
    smsc_log_debug(logger, "Session create: count=%d, addr=%s, usr=%d, uid=%d", sc, key.abonentAddr.toString().c_str(), key.USR, uid);

    m_SessionKey = key;
}

Session::~Session()
{
    // rollback all pending billing transactions & destroy session
    {
        uint32_t sc = 0;
        {
            MutexGuard mtxx(cntMutex);
            sc = --sessionCounter;
        }
        smsc_log_debug(logger, "Session destroy: count=%d, addr=%s, usr=%d, uid=%d", sc, m_SessionKey.abonentAddr.toString().c_str(), m_SessionKey.USR, uid);
        if(cmdQueue.Count())
        {
            smsc_log_debug(logger, "!!!Session command queue not empty: %d", cmdQueue.Count());        
            abort();
        }
    }

    char * key;
    AdapterProperty * value = 0;

    PropertyHash.First();
    for (Hash <AdapterProperty *>::Iterator it = PropertyHash.getIterator(); it.Next(key, value);)
        if (value) delete value;

    ClearOperations();
    //smsc_log_debug(logger,"Session deleted (%s,%d)", m_SessionKey.abonentAddr.toString().c_str(),m_SessionKey.USR);

}


void Session::DeserializeProperty(SessionBuffer& buff)
{
    uint32_t Count;
    buff >> Count;

    for (int i=0; i < Count; i++)
    {
        AdapterProperty * property = new AdapterProperty(this);
        buff >> *property;
        PropertyHash.Insert(property->GetName().c_str(), property);
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
        operation = new Operation(this);

        uint8_t temp;

        buff >> temp;
        operation->m_receivedAllResp = temp;
        buff >> operation->m_receivedResp;

        buff >> temp;
        operation->m_receivedAllParts = temp;
        buff >> operation->m_receivedParts;

        buff >> operation->flags;

        buff >> temp;
        operation->m_hasBill = temp;

        if (operation->m_hasBill) buff >> operation->billId;


        buff >> operation->type;
        buff >> key;
        OperationsHash.Insert(key,operation);
        //smsc_log_debug(logger, "DESERIALIZE ALLRESP=%d, ALLPARTS=%d",operation->m_receivedAllResp, operation->m_receivedAllParts);
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

        buff >> p.billID;

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
    int key;
    COperationsHash::Iterator it = OperationsHash.First();

    buff << OperationsHash.Count();

    for (;it.Next(key, operation);)
    {
        uint8_t temp;
        temp = operation->m_receivedAllResp;
        //smsc_log_debug(logger, "SERIALIZE ALLRESP=%d",temp);

        buff << temp;
        buff << operation->m_receivedResp;

        temp =  operation->m_receivedAllParts;
        buff << temp;
        buff << operation->m_receivedParts;
        //smsc_log_debug(logger, "SERIALIZE ALLPARTS=%d",temp);

        buff << operation->flags;

        temp = operation->m_hasBill;
        buff << temp;

        if (operation->m_hasBill) buff << operation->billId;

        buff << operation->type;
        buff << key;
        //smsc_log_debug(logger, "SERIALIZE ALLRESP=%d, ALLPARTS=%d",operation->m_receivedAllResp, operation->m_receivedAllParts);
    }
}

void Session::SerializePendingOperations(SessionBuffer& buff)
{
    std::list<PendingOperation>::iterator it;

    buff << (int)(PendingOperationList.size());

    for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
    {
        int temp = it->bStartBillingOperation;
        buff << it->type << it->validityTime << temp;

        buff << it->billID;

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

    buff << m_SessionKey.abonentAddr << (uint32_t)m_SessionKey.USR << lastAccessTime << lastOperationId;
    //buff << m_SmppDiscriptor.cmdType << m_SmppDiscriptor.currentIndex << m_SmppDiscriptor.lastIndex;
    buff << m_SessionPrimaryKey.sAddr.c_str();
  buff << ruleKey.serviceId;
  buff << ruleKey.transport;
  buff << (uint32_t)bIsNew;
}


void Session::Deserialize(SessionBuffer& buff)
{
    PrePendingOperationList.clear();
    m_bRedirectFlag = false;

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

    uint32_t tmp;
    buff >> tmp;
    m_SessionKey.USR = tmp;

    uint8_t c;
    buff >> lastAccessTime >> lastOperationId;
    // >> c;
    //m_isTransact = c;

    //buff >> tmp;
    //m_SmppDiscriptor.cmdType = (CommandOperations)tmp;

    //buff >> tmp;
    //m_SmppDiscriptor.currentIndex = tmp;

    //buff >> tmp;
    //m_SmppDiscriptor.lastIndex = tmp;

    buff >> m_SessionPrimaryKey.sAddr;

  buff >> ruleKey.serviceId;
  buff >> ruleKey.transport;
  buff >> tmp;
  bIsNew = tmp;
}



void Session::changed(AdapterProperty& property)
{
 /*   AdapterProperty * oldProperty = PropertyHash.Get(property.GetName().c_str());

    oldProperty->setPureStr(property.getStr());*/
    bChanged = true;
}

Property* Session::getProperty(const std::string& name)
{
    AdapterProperty ** propertyPTR = PropertyHash.GetPtr(name.c_str());
    AdapterProperty * property = 0;

    if (name=="ICC_STATUS")
    {
        if (!m_pCurrentOperation) return 0;

        if (!propertyPTR)
        {
            property = new AdapterProperty(name,this, m_pCurrentOperation->getStatus());
            PropertyHash.Insert(name.c_str(),property);
            return property;
        }

        (*propertyPTR)->setInt(m_pCurrentOperation->getStatus());

        return (*propertyPTR);
    }

    if (name=="USR")
    {
        if (!propertyPTR)
        {
            property = new AdapterProperty(name,this, m_SessionKey.USR);
            PropertyHash.Insert(name.c_str(),property);
            return property;
        }

        return (*propertyPTR);
    }

    if (name=="abonent")
    {
        if (!propertyPTR)
        {
            property = new AdapterProperty(name, this, m_SessionKey.abonentAddr.toString());
            PropertyHash.Insert(name.c_str(), property);
            return property;
        }

        return (*propertyPTR);
    }

    if (propertyPTR) return (*propertyPTR);

    property = new AdapterProperty(name,this,"");
    PropertyHash.Insert(name.c_str(),property);

    return property;
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
    bChanged = true;
}

void Session::abort()
{
    int key;
    Operation * value;

    COperationsHash::Iterator it = OperationsHash.First();
    smsc_log_debug(logger,"Session: abort (usr=%d, %s)", m_SessionKey.USR, m_SessionKey.abonentAddr.toString().c_str());

    for (;it.Next(key, value);)
    {
        value->rollbackAll();
        delete value;
    }

    for (std::list<PendingOperation>::iterator it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
    {
        it->rollbackAll();
    }


    OperationsHash.Empty();
    PendingOperationList.clear();
    PrePendingOperationList.clear();

    m_pCurrentOperation = 0;
    lastOperationId = 0;
    bChanged = true;

    smsc_log_debug(logger,"Session: session aborted (usr=%d, %s)", m_SessionKey.USR, m_SessionKey.abonentAddr.toString().c_str());
}

bool Session::hasOperations()
{
    return !(PendingOperationList.empty() && (OperationsHash.Count() == 0));
}

int Session::getNewOperationId()
{
    bChanged = true;
    return ++lastOperationId;
}

bool Session::hasPending()
{
    return (PendingOperationList.size() > 0);
}

bool Session::expirePendingOperation()
{
    bool reorder = false;
    if (PendingOperationList.size() > 0)
    {
        std::list<PendingOperation>::iterator it = PendingOperationList.begin();
        time_t now = time(NULL);
        while(it != PendingOperationList.end() && it->validityTime <= now)
        {
            if (it->billID > 0) it->rollbackAll(true);

            smsc_log_debug(logger,"Session: pending operation has expiried (billId = %d, type=%d, ab=%s)",it->billID, it->type, m_SessionKey.abonentAddr.toString().c_str());
            it++;
        }
        if(it != PendingOperationList.begin())
        {
            PendingOperationList.erase(PendingOperationList.begin(), it);
            reorder= true;
        }
        bChanged = true;
    }
    return reorder;
}


void Session::closeCurrentOperation()
{
    if (!m_pCurrentOperation) return;

    smsc_log_debug(logger,"Session: close current operation (id=%lld, type=%d, ab=%s, billId=%d)", currentOperationId, m_pCurrentOperation->type, m_SessionKey.abonentAddr.toString().c_str(), m_pCurrentOperation->billId);

    /*delete m_pCurrentOperation;
    m_pCurrentOperation = 0;
    getOperationsHash()->Delete(currentOperationId);*/

    Operation ** opPtr = OperationsHash.GetPtr(currentOperationId);
    if (opPtr)
    {
       (*opPtr)->rollbackAll();
        delete (*opPtr);

        OperationsHash.Delete(currentOperationId);
        m_pCurrentOperation = 0;
        smsc_log_debug(logger,"Session: current operation (id=%lld) released (count = %d)", currentOperationId, OperationsHash.Count());
    }


    bChanged = true;
}


Operation * Session::setCurrentOperation(uint64_t operationId)
{
    Operation ** operationPtr = OperationsHash.GetPtr(operationId);
    if (!operationPtr) throw SCAGException("Cannot find operation (id=%lld, ab=%s)", operationId, m_SessionKey.abonentAddr.toString().c_str());

    currentOperationId = operationId;
    m_pCurrentOperation = (*operationPtr);

    smsc_log_debug(logger,"Session: set current operation (id=%lld), type=%d, billId=%d", operationId, m_pCurrentOperation->type, m_pCurrentOperation->getBillId());

    bChanged = true;
    return (*operationPtr);
}

Operation * Session::setCurrentOperationByType(int operationType)
{
    Operation * operation;
    int key;

    COperationsHash::Iterator it = OperationsHash.First();

    for (;it.Next(key, operation);)
    {
        if (operation->type == operationType)
        {
            bChanged = true;
            currentOperationId = key;
            m_pCurrentOperation = operation;

            return operation;
        }
    }

    throw SCAGException("Session: Cannot find operation with type=%d, ab=%s", operationType, m_SessionKey.abonentAddr.toString().c_str());
}


Operation * Session::setOperationFromPending(SCAGCommand& cmd, int operationType)
{
    std::list<PendingOperation>::iterator it;

    for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
    {
        if (it->type == operationType)
        {
            Operation * operation = AddNewOperationToHash(cmd, operationType);

            if (it->billID > 0)
                operation->attachBill(it->billID);

            smsc_log_debug(logger,"** Session: pending closed (type=%d, ab=%s), billId=%d", it->type, m_SessionKey.abonentAddr.toString().c_str(), it->billID);

            PendingOperationList.erase(it);
            bChanged = true;

            return operation;
        }
    }
    throw SCAGException("Session: Cannot find pending operation (type=%d, ab=%s)", operationType, m_SessionKey.abonentAddr.toString().c_str());
}

Operation * Session::AddNewOperationToHash(SCAGCommand& cmd, int operationType)
{

    Operation * operation = new Operation(this);
    operation->type = operationType;

    cmd.setOperationId(getNewOperationId());
    OperationsHash.Insert(cmd.getOperationId(),operation);
    currentOperationId = cmd.getOperationId();
    m_pCurrentOperation = operation;

    bChanged = true;

    smsc_log_debug(logger,"** Session: create new operation (id=%lld, type=%d, ab=%s)", currentOperationId, operationType, m_SessionKey.abonentAddr.toString().c_str());

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
            bChanged = true;
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
    if (PendingOperationList.empty()) return time(NULL);
    return (PendingOperationList.begin())->validityTime;
}

uint64_t Session::getCurrentOperationId()
{
    return currentOperationId;
}

}}
