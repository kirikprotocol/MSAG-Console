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


void Session::endOperation(RuleStatus& ruleStatus)
{
    if (!m_pCurrentOperation) throw SCAGException("Session: Fatal error - cannot end operation. Couse: current operation not found");

    switch (m_SmppDiscriptor.cmdType)
    {
    case CO_DELIVER_SM:
        //smsc_log_debug(logger,"Session: CO_DELIVER_SM detected");
        break;

    case CO_DELIVER_SM_RESP:
        //smsc_log_debug(logger,"Session: CO_DELIVER_SM_RESP detected - lastIndex %d, currentIndex %d",m_SmppDiscriptor.lastIndex,m_SmppDiscriptor.currentIndex);

        if ((m_SmppDiscriptor.lastIndex == 0)||((m_SmppDiscriptor.lastIndex > 0)&&(m_pCurrentOperation->hasReceivedAllResp())))
        {
            closeCurrentOperation();
        }
        break;

    case CO_SUBMIT_SM:
        //smsc_log_debug(logger,"Session: CO_SUBMIT_SM detected");

        if (!m_isTransact) 
        {
            time_t now;
            time(&now);

            PendingOperation pendingOperation;
            pendingOperation.type = CO_RECEIPT_DELIVER_SM;
            pendingOperation.validityTime = now + SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;

            PrePendingOperationList.push_back(pendingOperation);
        }
        break;

    case CO_SUBMIT_SM_RESP:
        //smsc_log_debug(logger,"Session: CO_SUBMIT_SM_RESP detected");

        if ((m_SmppDiscriptor.lastIndex = 0)||((m_SmppDiscriptor.lastIndex > 0)&&(m_pCurrentOperation->hasReceivedAllResp())))
        {
            closeCurrentOperation();
        }
        break;

    case CO_RECEIPT_DELIVER_SM:
        //smsc_log_debug(logger,"Session: CO_RECEIPT_DELIVER_SM detected");
        //TODO:: ����� ������ �������� ��� multipart
        closeCurrentOperation();
        break;


    case CO_USSD_DELIVER_RESP:
            smsc_log_debug(logger,"Session: finish process USSD_DELIVER_RESP");
            if (m_SmppDiscriptor.isUSSDClosed) closeCurrentOperation();
            break;

    case CO_USSD_SUBMIT_RESP:
            smsc_log_debug(logger,"Session: finish process USSD_SUBMIT_RESP");

            if (m_SmppDiscriptor.isUSSDClosed) closeCurrentOperation();
            break;

    }
    //smsc_log_error(logger,"** Session: operation end, pendinding count = %d - %d",PendingOperationList.size(),PrePendingOperationList.size());
}

Operation * Session::AddNewOperationToHash(SCAGCommand& cmd, int type)
{
    smsc_log_debug(logger,"** Session: create new operation");

    Operation * operation = new Operation();
    operation->type = type;

    cmd.setOperationId(getNewOperationId());
    OperationsHash.Insert(cmd.getOperationId(),operation);
    currentOperationId = cmd.getOperationId();
    m_pCurrentOperation = operation;

    bChanged = true;
    return operation;
}

bool Session::startOperation(SCAGCommand& cmd)
{
    Operation * operation = 0;

    try
    {
        m_SmppDiscriptor = CommandBrige::getSmppDiscriptor(cmd);
    } catch (SCAGException& e)
    {
        throw e;
    }

    switch (m_SmppDiscriptor.cmdType)
    {
    case CO_DELIVER_SM:
        {
            //smsc_log_debug(logger,"Session: DELIVER operation, currIndex %d, lastIndex %d",m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);

            Operation ** operationPtr;

            if ((m_SmppDiscriptor.currentIndex == 0)&&(m_SmppDiscriptor.lastIndex == 0))
            {
                AddNewOperationToHash(cmd, m_SmppDiscriptor.cmdType);
            } else
            {
                operationPtr = OperationsHash.GetPtr(cmd.getOperationId());

                if (!operationPtr) 
                    operation = AddNewOperationToHash(cmd, m_SmppDiscriptor.cmdType);
                else 
                    operation = *operationPtr;

                operation->receiveNewPart(m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);

                currentOperationId = cmd.getOperationId();
                m_pCurrentOperation = operation;
                bChanged = true;
            }
                
            break;
        }

    case CO_DELIVER_SM_RESP:
        {
            //smsc_log_debug(logger,"Session: DELIVER_RESP operation, currIndex %d, lastIndex %d",m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);

            Operation ** operationPtr;

            operationPtr = OperationsHash.GetPtr(cmd.getOperationId());

            if (!operationPtr) 
            {
                smsc_log_debug(logger,"Session: no operation matched to CO_DELIVER_SM_RESP has been detected");
                return false;
            }
            operation = *operationPtr;

            bChanged = true;

            currentOperationId = cmd.getOperationId();
            m_pCurrentOperation = operation;

            operation->receiveNewResp(m_SmppDiscriptor.currentIndex, m_SmppDiscriptor.lastIndex);

            break;
        }

    case CO_SUBMIT_SM:
        {
            //smsc_log_debug(logger,"Session: SUBMIT operation, currIndex %d, lastIndex %d",m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);

            int UMR = CommandBrige::getUMR(cmd);
            Operation ** operationPtr = 0;

            if (UMR == 0)
            {
                if (m_SmppDiscriptor.currentIndex == 0) AddNewOperationToHash(cmd, m_SmppDiscriptor.cmdType);
                else
                {
                    operationPtr = OperationsHash.GetPtr(cmd.getOperationId());
                    if (!operationPtr) AddNewOperationToHash(cmd, m_SmppDiscriptor.cmdType);
                }
                break;
            }
            //TODO: ��������� ���� �� ��������� �������� SUBMIT, ���� � ���, �� ��� � ��� ������?

            //single command
            if (m_SmppDiscriptor.currentIndex == 0)
            {
                std::list<PendingOperation>::iterator it;
    
                for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
                {
                    if (it->type == m_SmppDiscriptor.cmdType) 
                    {
                        AddNewOperationToHash(cmd, m_SmppDiscriptor.cmdType);
                        PendingOperationList.erase(it);
                        break;
                    }
                }
                smsc_log_debug(logger,"Session: no pending operation found");
                return false;
            } 

            //multipart command
            if (m_SmppDiscriptor.lastIndex > 0) 
            {
                operationPtr = OperationsHash.GetPtr(cmd.getOperationId());

                if (!operationPtr) 
                {
                    smsc_log_debug(logger,"Session: no operation matched to CO_SUBMIT_SM");
                    return false;
                }

                operation = *operationPtr;

                currentOperationId = cmd.getOperationId();
                m_pCurrentOperation = operation;
                operation->receiveNewPart(m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);
                bChanged = true;
            }
            break;
        }

    case CO_SUBMIT_SM_RESP:
        {
            //smsc_log_debug(logger,"Session: SUBMIT_RESP operation, currIndex %d, lastIndex %d",m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);

            Operation ** operationPtr = OperationsHash.GetPtr(cmd.getOperationId());

            if (!operationPtr) 
            {
                smsc_log_debug(logger,"Session: no operation matched to CO_SUBMIT_SM_RESP");
                return false;
            }

            operation = *operationPtr;
            
            currentOperationId = cmd.getOperationId();
            m_pCurrentOperation = operation;

            bChanged = true;

            operation->receiveNewResp(m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);
            break;
        }

    case CO_RECEIPT_DELIVER_SM:
        {
            /*Operation ** operationPtr = OperationsHash.GetPtr(cmd.getOperationId());

            if (!operationPtr) 
            {
                smsc_log_debug(logger,"Session: no operation matched to CO_RECEIPT_DELIVER_SM");
                return false;
            } */

            std::list<PendingOperation>::iterator it;

            for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
            {
                if (it->type == CO_RECEIPT_DELIVER_SM) 
                {
                    AddNewOperationToHash(cmd, CO_RECEIPT_DELIVER_SM);
                    PendingOperationList.erase(it);
                    break;
                }
                smsc_log_debug(logger,"Session: no pending operations found for CO_RECEIPT_DELIVER_SM");
                return false;
            }
            break;
        }


    case CO_USSD_DELIVER:
        {
            smsc_log_debug(logger,"Session: process USSD_DELIVER");

            if (m_SmppDiscriptor.wantOpenUSSD) AddNewOperationToHash(cmd, CO_USSD_DELIVER);
            break;
        }

    case CO_USSD_SUBMIT:
        {
            smsc_log_debug(logger,"Session: process USSD_SUBMIT");

            if (m_SmppDiscriptor.wantOpenUSSD) AddNewOperationToHash(cmd, CO_USSD_SUBMIT);
            break;
        }

    case CO_USSD_DELIVER_RESP:
        {

            break;
        }

    case CO_USSD_SUBMIT_RESP:
        {

            break;
        }

    }
    //smsc_log_error(logger,"** Session: operation started, Pending: %d-%d, Operations: %d",PendingOperationList.size(),PrePendingOperationList.size(), OperationsHash.Count());
   
    return true;
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