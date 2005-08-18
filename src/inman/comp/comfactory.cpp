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
{
	REG_PRODUCT( InitialDPSMS );
	/*
	still abstract
	REG_PRODUCT( RequestReportSMSEvent );
	REG_PRODUCT( EventReportSMS );
	*/
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

}
}
}