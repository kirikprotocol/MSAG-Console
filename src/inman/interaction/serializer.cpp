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
	registerProduct( CHARGE_SMS_TAG, new ProducerT< ChargeSms >() );
	registerProduct( CHARGE_SMS_RESULT_TAG, new ProducerT< ChargeSmsResult>() );
	registerProduct( DELIVERY_SMS_RESULT_TAG, new ProducerT< DeliverySmsResult>() );
}

Serializer::~Serializer()
{
}


SerializableObject* Serializer::deserialize(ObjectBuffer& in)
{
	USHORT_T objectId, version, dialogId;

	in >> version;

	if( version != FORMAT_VERSION ) 
		throw runtime_error( format("Invalid version: 0x%X", version) );

	in >> dialogId;

	in >> objectId;

	SerializableObject* obj = create( objectId );

	if( !obj ) 
		throw runtime_error( format("Invalid object ID: 0x%X", objectId) );


	obj->setDialogId( dialogId );

	obj->load( in );

	return obj;
}


void Serializer::serialize(SerializableObject* obj, ObjectBuffer& out)
{
	assert( obj );
	out << (USHORT_T) FORMAT_VERSION;
	out << (USHORT_T) obj->getDialogId();
	obj->save( out );
}

ObjectPipe::ObjectPipe(Socket* sock) 
	: socket( sock )
	, logger( Logger::getInstance("smsc.inman.interaction.ObjectPipe") )
{
	assert( socket );
}

ObjectPipe::~ObjectPipe()
{
}

SerializableObject* ObjectPipe::receive()
{
	char buf[1024];

  	int n = socket->Read(buf, sizeof(buf) - 1);
  	if( n < 1 )
  	{
  		smsc_log_error( logger, "Can't read from socket" );
  		return NULL;
  	}

  	ObjectBuffer buffer( n );
  	buffer.Append( buf, n );
  	buffer.SetPos( 0 );
	smsc_log_debug(logger, "Recv: %s", dump( n, (unsigned char*)buf ).c_str(), true );
	return Serializer::getInstance()->deserialize( buffer );
}

void ObjectPipe::send(SerializableObject* obj)
{
	assert( obj );
	ObjectBuffer buffer(16);
	Serializer::getInstance()->serialize( obj, buffer );
	socket->Write( buffer.get(), buffer.GetPos() );
	smsc_log_debug(logger, "Send: %s", dump( buffer.GetPos(), (unsigned char*)buffer.get() ).c_str(), true );
}


Serializer* Serializer::getInstance()
{
	static Serializer instance;
	return &instance;
}



} // namespace inap
} // namespace inmgr
} // namespace smsc
