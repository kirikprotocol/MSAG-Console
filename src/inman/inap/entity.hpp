/* ************************************************************************* *
 * Transaction Component with local operation tag (no objId as opcode).
 * ************************************************************************* */
#ifndef __SMSC_INMAN_TCAP_ENTITY__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TCAP_ENTITY__

#include "inman/comp/compdefs.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

using smsc::inman::comp::Component;

typedef std::vector<unsigned char> RawBuffer;

//Transaction Component
class TcapEntity {
public:
  enum TCEntityKind {
    tceNone = 0, tceInvoke, tceResult, tceResultNL, tceError
  };

  explicit TcapEntity(uint8_t inv_id, TCEntityKind e_kind = tceNone, uint8_t op_code = 0)
    : _invId(inv_id), _ekind(e_kind), _opCode(op_code), _param(NULL), _ownComp(false)
  { }
  virtual ~TcapEntity()
  {
    if (_ownComp) delete _param;
  }

  TCEntityKind kind() const           { return _ekind; }
  uint8_t     getId() const           { return _invId; }
  uint8_t     getOpcode() const       { return _opCode; }
  Component*  getParam() const        { return _param; }
  //sets '_param' without passing ownership, it's caller responsibility to free Component
  void        setParam(Component * p)  { _param = p; _ownComp = false; }
  //grands the ownership of '_param', Component will be freed by ~TcapEntity()
  void        ownParam(Component * p)  { _param = p; _ownComp = true; }

  //throws CustomException
  void encode(RawBuffer & operation, RawBuffer & params) const throw(CustomException)
  {
    operation.clear(); params.clear();
    operation.push_back(_opCode);
    if (_param)
      _param->encode(params);
  }

protected:
  uint8_t       _invId;   //id of associated Invocation 
  TCEntityKind  _ekind;   //ROS Component kind
  uint8_t       _opCode;  //Operation code of associated Invocation 
  Component *	_param;
  bool          _ownComp; //this Entity is owner of 'param'
};

class TCResult : public TcapEntity {
public:
  TCResult(uint8_t inv_id, uint8_t op_code)
    : TcapEntity(inv_id, TcapEntity::tceResult, op_code)
  { }
};

class TCResultNL : public TcapEntity {
public:
  TCResultNL(uint8_t inv_id, uint8_t op_code)
    : TcapEntity(inv_id, TcapEntity::tceResultNL, op_code)
  { }
};

class TCError : public TcapEntity {
public:
  TCError(uint8_t inv_id, uint8_t op_code)
    : TcapEntity(inv_id, TcapEntity::tceError, op_code)
  { }
};

} //inap
} //inman
} //smsc
#endif /* __SMSC_INMAN_TCAP_ENTITY__ */

