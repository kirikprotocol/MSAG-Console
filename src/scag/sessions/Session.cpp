#include "Session.h"

namespace scag { namespace sessions {

void Session::changed(AdapterProperty& property)
{
    AdapterProperty * oldProperty = PropertyHash.Get(property.GetName().c_str());
    oldProperty->setStr(property.getStr());
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
        bChanged(false), bDestroy(false), accessCount(0)
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
}

bool Session::Aquire()
{
    time_t accessTime = time(NULL);
    MutexGuard guard(accessMonitor);
    while (accessCount > 0) accessMonitor.Wait();
    if (bDestroy) {
        if (accessCount <= 0) delete this;
        return false;
    }
    lastAccessTime = accessTime;
    accessCount++;
    return true;
}
void Session::Release()
{
    MutexGuard guard(accessMonitor);
    if (accessCount > 0) {
        --accessCount;
        accessMonitor.NotifyAll();
    }
}
void Session::Destroy()
{
    MutexGuard guard(accessMonitor);
    if (accessCount > 0) {
        bDestroy = true;
        --accessCount;
        accessMonitor.NotifyAll();
    }
    else delete this;
}
void Session::Expire()
{
    // TODO: rollback all pending billing transactions & destroy session
}


}}