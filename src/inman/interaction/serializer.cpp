static char const ident[] = "$Id$";
#include <assert.h>
#include <stdexcept>

#include "serializer.hpp"
#include "messages.hpp"

using std::runtime_error;

namespace smsc  {
namespace inman {
namespace interaction  {

Serializer::Serializer()
{
	registerProduct( StartMessage::OBJECT_ID, new ProducerT< StartMessage >() );
}

Serializer::~Serializer()
{
}

SerializableObject* Serializer::deserialize(ObjectBuffer& in)
{
	USHORT_T objectId, version;

	in >> version;

	if( version != FORMAT_VERSION ) 
		throw runtime_error( format("Invalid version: 0x%X", version) );

	in >> objectId;

	SerializableObject* obj = create( objectId );

	if( !obj ) 
		throw runtime_error( format("Invalid object ID: 0x%X", objectId) );

	return obj;
}


void Serializer::serialize(SerializableObject* obj, ObjectBuffer& out)
{
	assert( obj );
	out << (USHORT_T) FORMAT_VERSION;
	obj->save( out );
}


Serializer* Serializer::getInstance()
{
	static Serializer instance;
	return &instance;
}



} // namespace inap
} // namespace inmgr
} // namespace smsc
