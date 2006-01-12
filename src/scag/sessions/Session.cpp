#include "Session.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag { namespace sessions {

using namespace scag::exceptions;
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
    int key;

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
    int key;

    COperationsHash::Iterator it = OperationsHash.First();

    buff << OperationsHash.Count();

    for (;it.Next(key, operation);)
    {              
        buff << operation->BillList.size();

        for (std::list<int>::iterator billIt = operation->BillList.begin();billIt!=operation->BillList.end(); ++billIt)
        {
            buff << (*billIt);
        }

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
        buff << currentOperationId;
   
    buff << m_SessionKey.abonentAddr << (uint32_t)m_SessionKey.USR << lastAccessTime << lastOperationId << m_isTransact;
    buff << m_SmppDiscriptor.cmdType << m_SmppDiscriptor.currentIndex << m_SmppDiscriptor.lastIndex;
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

    buff >> lastAccessTime >> lastOperationId >> m_isTransact;     

    buff >> tmp;
    m_SmppDiscriptor.cmdType = (CommandOperations)tmp;

    buff >> tmp;
    m_SmppDiscriptor.currentIndex = tmp;

    buff >> tmp;
    m_SmppDiscriptor.lastIndex = tmp;
}



void Session::changed(AdapterProperty& property)
{
    AdapterProperty * oldProperty = PropertyHash.Get(property.GetName().c_str());
    oldProperty->setPureStr(property.getStr());
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

Session::Session(const CSessionKey& key) 
    : PropertyManager(), lastAccessTime(-1), 
        bChanged(false), bDestroy(false), accessCount(0), Owner(0),m_pCurrentOperation(0),
        logger(0), lastOperationId(0), m_isTransact(false)
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

    ClearOperations();
    smsc_log_debug(logger,"Session: session destroyed");
}


void Session::ClearOperations()
{
    if (!Owner) return;

    int key;
    Operation * value;

    COperationsHash::Iterator it = OperationsHash.First();

    for (;it.Next(key, value);)
    {              
        delete value;
    }

    OperationsHash.Empty();
    PendingOperationList.clear();

    m_pCurrentOperation = 0;
    lastOperationId = 0;
}

void Session::abort()
{
    ClearOperations();
    Owner->startTimer(m_SessionKey,0);

    smsc_log_error(logger,"Session: session aborted");
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
    return !(PendingOperationList.empty());
}

void Session::expirePendingOperation()
{
    if (!(PendingOperationList.empty())) 
    {
        PendingOperationList.pop_front();
        smsc_log_debug(logger,"Session: pending operation has expiried");
    }
}


void Session::closeCurrentOperation()
{
    delete m_pCurrentOperation;
    m_pCurrentOperation = 0;
    OperationsHash.Delete(currentOperationId);
    smsc_log_debug(logger,"Session: close current operation");
}

void Session::endOperation(RuleStatus& ruleStatus)
{
    if (!m_pCurrentOperation) throw SCAGException("Session: Fatal error - cannot end operation. Couse: current operation not found");

    Operation * operation = 0;

    switch (m_SmppDiscriptor.cmdType)
    {
    case CO_DELIVER_SM:
        smsc_log_debug(logger,"Session: CO_DELIVER_SM detected");
        break;

    case CO_DELIVER_SM_RESP:
        smsc_log_debug(logger,"Session: CO_DELIVER_SM_RESP detected %d-%d",m_SmppDiscriptor.lastIndex,m_SmppDiscriptor.currentIndex);

        if ((m_SmppDiscriptor.lastIndex == 0)||((m_SmppDiscriptor.lastIndex > 0)&&(m_SmppDiscriptor.lastIndex == m_SmppDiscriptor.currentIndex)))
        {
            closeCurrentOperation();
        }
        break;

    case CO_SUBMIT_SM:
        smsc_log_debug(logger,"Session: CO_SUBMIT_SM detected");

        if (!m_isTransact) 
        {
            PendingOperation pendingOperation;
            pendingOperation.type = CO_RECEIPT_DELIVER_SM;
            pendingOperation.validityTime = SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;

            PendingOperationList.push_back(pendingOperation);
        }
        break;

    case CO_SUBMIT_SM_RESP:
        smsc_log_debug(logger,"Session: CO_SUBMIT_SM_RESP detected");

        if ((m_SmppDiscriptor.lastIndex = 0)||((m_SmppDiscriptor.lastIndex > 0)&&(m_SmppDiscriptor.lastIndex == m_SmppDiscriptor.currentIndex)))
        {
            closeCurrentOperation();
        }
        break;

    case CO_RECEIPT_DELIVER_SM:
        smsc_log_debug(logger,"Session: CO_RECEIPT_DELIVER_SM detected");
        //TODO:: Ќужно учесть политику дл€ multipart
        closeCurrentOperation();
        break;

    }

}

void Session::AddNewOperationToHash(SCAGCommand& cmd, int type)
{
    Operation * operation = new Operation();
    operation->type = type;

    cmd.setOperationId(getNewOperationId());
    OperationsHash.Insert(cmd.getOperationId(),operation);
    currentOperationId = cmd.getOperationId();
    m_pCurrentOperation = operation;
}

bool Session::startOperation(SCAGCommand& cmd)
{

    if (!Owner) return false;

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

            if (m_SmppDiscriptor.currentIndex == 0)
            {
                AddNewOperationToHash(cmd, m_SmppDiscriptor.cmdType);
            } else if (m_SmppDiscriptor.lastIndex > 0) 
            {
                operation = OperationsHash.Get(cmd.getOperationId());
                //TODO: check what to do if there are no session?
                //if (!operation) ...
                operation->setStatus(m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);


                currentOperationId = cmd.getOperationId();
                m_pCurrentOperation = operation;
            }
                
            break;
        }

    case CO_DELIVER_SM_RESP:
        {
            smsc_log_debug(logger,"Session: !! CO_DELIVER_SM_RESP detected !!");

            operation = OperationsHash.Get(cmd.getOperationId());
            //TODO: check what to do if there are no session?
            //if (!operation) ...

            currentOperationId = cmd.getOperationId();
            m_pCurrentOperation = operation;

            if (m_SmppDiscriptor.lastIndex == 0) break; //single response

            //multipart response
            operation->setStatus(m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);
            break;
        }

    case CO_SUBMIT_SM:
        {

            int UMR = CommandBrige::getUMR(cmd);
            if (UMR == 0)
            {
                AddNewOperationToHash(cmd, m_SmppDiscriptor.cmdType);
                break;
            }
            //TODO: проверить есть ли ожидаема€ операци€ SUBMIT, если еЄ нет, то что и как делать?

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
            } 
            //multipart command
            if (m_SmppDiscriptor.lastIndex > 0) 
            {
                operation = OperationsHash.Get(cmd.getOperationId());
                //TODO: check what to do if there are no session?
                //if (!operation) ...

                currentOperationId = cmd.getOperationId();
                m_pCurrentOperation = operation;
                operation->setStatus(m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);
            }
            break;
        }

    case CO_SUBMIT_SM_RESP:
        {

            operation = OperationsHash.Get(cmd.getOperationId());
            //TODO: check what to do if there are no session?
            //if (!operation) ...
            currentOperationId = cmd.getOperationId();
            m_pCurrentOperation = operation;

            if (m_SmppDiscriptor.lastIndex == 0) break; //single response

            operation->setStatus(m_SmppDiscriptor.currentIndex,m_SmppDiscriptor.lastIndex);
            break;
        }

    case CO_RECEIPT_DELIVER_SM:
        {
        }


    }
    smsc_log_error(logger,"** Session: operation started");
   
    //Owner->startTimer(this->getSessionKey(), this->getWakeUpTime());
    return true;
}

void Session::addPendingOperation(PendingOperation pendingOperation)
{

    std::list<PendingOperation>::iterator it;

    for (it = PendingOperationList.begin(); it!=PendingOperationList.end(); ++it)
    {
        if (it->validityTime > pendingOperation.validityTime) 
        {
            PendingOperationList.insert(it,pendingOperation);
            Owner->startTimer(this->getSessionKey(), this->getWakeUpTime());
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