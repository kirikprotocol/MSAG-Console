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

Session::~Session()
{
    char * key;
    AdapterProperty * value = 0;

    PropertyHash.First();
    for (Hash <AdapterProperty *>::Iterator it = PropertyHash.getIterator(); it.Next(key, value);)
    {
        delete value;
    }
}


}}