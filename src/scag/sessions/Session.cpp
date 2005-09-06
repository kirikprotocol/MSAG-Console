#include "Session.h"
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
        bChanged(false), bDestroy(false), accessCount(0), Owner(0),currentOperation(0)
{
    // TODO: ???
}

Session::~Session()
{
    char * key;
    AdapterProperty * value = 0;

    PropertyHash.First();
    for (Hash <AdapterProperty *>::Iterator it = PropertyHash.getIterator(); it.Next(key, value);)
        if (value) delete value;


    std::list<Operation *>::const_iterator it;

    for (it = OperationList.begin(); it!=OperationList.end(); ++it)
    {
        delete (*it);
    }
}

void Session::Expire()
{
    // TODO: rollback all pending billing transactions & destroy session
}


bool Session::hasOperations() const
{
    return !(PendingOperationList.empty() && OperationList.empty());
}

void Session::expireOperation(time_t currentTime)
{

}

bool Session::startOperation(SCAGCommand& cmd)
{
    if (!Owner) return false;

    //TODO: add Operation
    //currentOperation = 
    
    Owner->startTimer(this->getSessionKey(), this->getWakeUpTime());
    return true;
}

void Session::releaseOperation()
{

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
    if (!OperationList.empty()) 
    {
        operation = *(OperationList.begin());
        time2 = operation->validityTime;
    }

    if (time1 == 0) return time2;
    if (time2 == 0) return time1;

    if (time1 > time2) return time2;
    else return time1;
}


}}