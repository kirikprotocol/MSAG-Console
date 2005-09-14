#include "Session.h"
#include "scag/SAX2Print.hpp"

#include <iostream>


namespace scag { namespace sessions {

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

Session::Session() 
    : PropertyManager(), lastAccessTime(-1), 
        bChanged(false), bDestroy(false), accessCount(0), Owner(0),currentOperation(0),needReleaseCurrentOperation(false)
{
    // TODO: ???

    //��������� ������, ��� ����, ����� SessionManager ������� �������� ������
    PendingOperation operation;
    time_t now;

    time(&now);

    operation.type = 1;
    operation.validityTime = now + 10;

    addPendingOperation(operation);
    //��������� ������, ��� ����, ����� SessionManager ������� �������� ������
}

Session::~Session()
{
    // TODO: rollback all pending billing transactions & destroy session


    char * key;
    AdapterProperty * value = 0;

    PropertyHash.First();
    for (Hash <AdapterProperty *>::Iterator it = PropertyHash.getIterator(); it.Next(key, value);)
        if (value) delete value;


    abort();
}


void Session::abort()
{
    COperationKey key;
    Operation * value;
    //OperationHash.getIterator()

    OperationHash.First();
    XHash <COperationKey,Operation *,XOperationHashFunc>::Iterator it = OperationHash.getIterator();

    for (;it.Next(key, value);)
    {
        delete value;
    }
   /*
    for (std::list<Operation *>::iterator it = OperationList.begin();it!=OperationList.end(); ++it)
    {
       //Operation * operation = (*it);
       //operation->abort();
       delete (*it);
    }
     */
    OperationHash.Empty();
    PendingOperationList.clear();
    smsc_log_error(logger,"session aborted");
}

bool Session::hasOperations() 
{
    return !(PendingOperationList.empty() && OperationHash.Count() == 0);
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

    if (!OperationHash.Count()) 
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