#include "Session.h"
#include "scag/exc/SCAGExceptions.h"
#include <time.h>

namespace scag { namespace sessions {

using namespace scag::exceptions;
using namespace scag::re;

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

int Session::getOperationType(std::string& str)
{
    int * pType = OperationTypesHash.GetPtr(str.c_str());
    if (!pType) throw SCAGException("Unknown OperationType '%s'", str.c_str());

    return (*pType);
}



void PendingOperation::rollbackAll()
{
    if (billID > 0) 
    {
        BillingManager& bm = BillingManager::Instance();

        __warning__("PendingOperation: Rollback all");

        try
        {
            bm.Rollback(billID);
        } catch (SCAGException& e)
        {
            __warning__("PendingOperation: Cannot rollback.");
            //smsc_log_debug(logger,"PendingOperation: Cannot rollback. Details: %s", e.what());
        }


        billID = 0;
    }
}




ICCOperationStatus Operation::getStatus()
{
    return m_Status;
}

void Operation::detachBill()
{
    if (!m_hasBill) 
    {
        smsc_log_warn(logger,"Operation: No bill to detach");
        return;
    }

    smsc_log_debug(logger,"Operation: Bill (id=%d) dettached", billId);
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


void Operation::attachBill(int BillId)
{ 
    if (m_hasBill) throw SCAGException("Operation: Cannot attach bill - bill already attached!");

    m_hasBill = true;
    billId = BillId;

    smsc_log_debug(logger,"Operation: Bill (id=%d) attached",BillId);
}


void Operation::rollbackAll()
{
    smsc_log_debug(logger,"Operation: Rollback all");
    
    if (m_hasBill) 
    {
        BillingManager& bm = BillingManager::Instance();

        try
        {
            bm.Rollback(billId);
        } catch (SCAGException& e)
        {
            smsc_log_debug(logger,"Operation: Cannot rollback. Details: %s", e.what());
        }


        m_hasBill = false;
    }
}

//////////////////////////////////////////////Session////////////////////////////////////////////

Session::Session(const CSessionKey& key) 
    : PropertyManager(), lastAccessTime(-1), 
        bChanged(false), bDestroy(false), accessCount(0), m_pCurrentOperation(0),
        logger(0), lastOperationId(0), m_CanOpenSubmitOperation(false), m_bRedirectFlag(false)
{
    logger = Logger::getInstance("scag.re");
    m_SessionKey = key;

    timeval tv;
    gettimeofday(&tv,0);

//    MillisecTime msec(tv);
//    smsc_log_debug(logger," *********** Session: msec %s (abonentAddr = '%s', USR=%d)", msec.toString(), m_SessionKey.abonentAddr.toString().c_str(),m_SessionKey.USR);
    

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
    //smsc_log_debug(logger,"Session deleted (%s,%d)", m_SessionKey.abonentAddr.toString().c_str(),m_SessionKey.USR);

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

        AdapterProperty * property = new AdapterProperty(name,this,value);
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

        uint8_t temp;

        buff >> temp;
        operation->m_receivedAllResp = temp;
        buff >> operation->m_receivedResp;

        buff >> temp;
        operation->m_receivedAllParts = temp;
        buff >> operation->m_receivedParts;

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
        std::string str = value->getStr();
        buff << str.c_str();
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
    buff << m_SessionPrimaryKey.abonentAddr << m_SessionPrimaryKey.BornMicrotime.tv_sec << m_SessionPrimaryKey.BornMicrotime.tv_usec;
    
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

    buff >> m_SessionPrimaryKey.abonentAddr;
    buff >> m_SessionPrimaryKey.BornMicrotime.tv_sec;
    buff >> m_SessionPrimaryKey.BornMicrotime.tv_usec;
                         
}



void Session::changed(AdapterProperty& property)
{
 /*   AdapterProperty * oldProperty = PropertyHash.Get(property.GetName().c_str());

    oldProperty->setPureStr(property.getStr());*/
    bChanged = true;
}

Property* Session::getProperty(const std::string& name)
{
    AdapterProperty ** propertyPTR = 0;

    if (name=="ICC_STATUS") 
    {
        if (!m_pCurrentOperation) return 0;

        propertyPTR = PropertyHash.GetPtr(name.c_str());

        if (!propertyPTR) 
        {

            AdapterProperty * property = new AdapterProperty(name,this, m_pCurrentOperation->getStatus());
            PropertyHash.Insert(name.c_str(),property);
            return property; 
        }
        (*propertyPTR)->setInt(m_pCurrentOperation->getStatus());

        return (*propertyPTR);

    }

    propertyPTR = PropertyHash.GetPtr(name.c_str());

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
    bChanged = true;
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

    smsc_log_debug(logger,"Session: session aborted");
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

void Session::expirePendingOperation()
{
    if (PendingOperationList.size() > 0) 
    {
        smsc_log_debug(logger,"Session: pending operation has expiried");

        std::list<PendingOperation>::iterator it = PendingOperationList.begin();
        if (it->billID > 0) it->rollbackAll();

        PendingOperationList.pop_front();
        bChanged = true;
    }
}


void Session::closeCurrentOperation()
{
    if (!m_pCurrentOperation) return;

    smsc_log_debug(logger,"Session: close current operation (id=%lld)", currentOperationId);

    /*delete m_pCurrentOperation;
    m_pCurrentOperation = 0;
    getOperationsHash()->Delete(currentOperationId);*/

    Operation ** opPtr = OperationsHash.GetPtr(currentOperationId);
    if (opPtr) 
    {
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
    if (!operationPtr) throw SCAGException("Cannot find operation (id=%lld)", operationId);


    currentOperationId = operationId;
    m_pCurrentOperation = (*operationPtr);

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

    throw SCAGException("Session: Cannot find operation with type=%d", operationType);
}


Operation * Session::setOperationFromPending(SCAGCommand& cmd, int operationType)
{
    std::list<PendingOperation>::iterator it;

    for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
    {
        if (it->type == operationType) 
        {
            Operation * operation = AddNewOperationToHash(cmd, operationType);

            if (it->billID > 0) operation->attachBill(it->billID);

            PendingOperationList.erase(it);
            bChanged = true;

            smsc_log_debug(logger,"** Session: pending closed", currentOperationId);
            return operation;
        }
    }
    throw SCAGException("Session: Cannot find pending operation (type=%d)", operationType);
}



Operation * Session::AddNewOperationToHash(SCAGCommand& cmd, int operationType)
{

    Operation * operation = new Operation();
    operation->type = operationType;

    cmd.setOperationId(getNewOperationId());
    OperationsHash.Insert(cmd.getOperationId(),operation);
    currentOperationId = cmd.getOperationId();
    m_pCurrentOperation = operation;

    bChanged = true;

    smsc_log_debug(logger,"** Session: create new operation (id=%lld)", currentOperationId);

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
    if (PendingOperationList.empty()) return 0;

    time_t time = (PendingOperationList.begin())->validityTime;
    return time;
}

uint64_t Session::getCurrentOperationId()
{
    return currentOperationId;
}

}}