#include <assert.h>

#include "comfactory.hpp"

#define REG_PRODUCT( name ) products.insert( ComponentMap::value_type( InapOpCode :: name, ProducerT< name##Arg >() ) );

namespace smsc {
namespace inman {
namespace comp{

// Do not add new fields to this template!          
template< class T >
struct ProducerT : public ComponentProducer
{
	virtual Component* create() const { return new T; }
};

ComponentFactory::ComponentFactory()
    : logger(Logger::getInstance("smsc.inman.comp.ComponentFactory"))
{
	REG_PRODUCT( InitialDPSMS );
	REG_PRODUCT( RequestReportSMSEvent );
}

ComponentFactory::~ComponentFactory()
{
}

Component* ComponentFactory::createComponent(unsigned opcode)
{
	ComponentMap::const_iterator it = products.find( opcode );
    if( it == products.end() )
    {
        return NULL;
    }
	return it->second.create();
}

ComponentFactory* ComponentFactory::getInstance()
{
	static ComponentFactory instance;
	return &instance;
}

}
}
}