#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_ENTITY__
#define __SMSC_INMAN_INAP_ENTITY__

#include "ss7cp.h"
#include "inman/common/types.hpp"
#include "inman/comp/comps.hpp"

using smsc::inman::comp::Component;

namespace smsc  {
namespace inman {
namespace inap  {

class TcapDialog;

class TcapEntity
{

public:

  TcapEntity()
	: id( 0 )
	, tag( 0 )
	, opcode( 0 )
	, param( NULL )
  {
  }
 
  virtual ~TcapEntity()
  {
  		delete param;
  }

  virtual USHORT_T getId() const 
  { 
  		return id; 
  }	

  virtual void setId( USHORT_T inId ) 
  { 
  		id = inId; 
  }

  virtual UCHAR_T getTag() const 
  { 
  		return tag; 
  }

  virtual void setTag(UCHAR_T t)
  {
  		tag = t;
  }

  virtual UCHAR_T getOpcode() const
  {
  		return opcode;
  }

  virtual void setOpcode(UCHAR_T opc)
  {
  		opcode = opc;
  }

  virtual Component* getParam() const
  {
		return param;
  }

  virtual void setParam(Component* p)
  {
		param = p;
  }

  virtual UCHAR_T send(TcapDialog*) = 0;

protected:

  typedef std::vector<unsigned char> RawBuffer;

  void encode(RawBuffer& operation, RawBuffer& params)
  {
  	operation.clear();
  	operation.push_back( opcode );
  	if( param ) param->encode( params );
  }

  UCHAR_T 		id;
  UCHAR_T		opcode;
  UCHAR_T  		tag;
  Component*	param;
};


}
}
}
#endif
