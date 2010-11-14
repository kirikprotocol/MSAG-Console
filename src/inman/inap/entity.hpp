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
    typedef enum {
        tceNone = 0, tceInvoke, tceResult, tceResultNL, tceError
    } TCEntityKind;

    TcapEntity(uint8_t tId, TCEntityKind e_kind = tceNone, uint8_t tOpCode = 0)
        : id(tId), ekind(e_kind), opcode(tOpCode), param(NULL), ownComp(false) { }

    virtual ~TcapEntity() { if (ownComp) delete param; }

    TCEntityKind kind() const           { return ekind; }
    uint8_t     getId() const           { return id; }
    uint8_t     getOpcode() const       { return opcode; }
    Component*  getParam() const        { return param; }
    //sets 'param' without passing ownership, it's caller responsibility to free Component
    void        setParam(Component* p)  { param = p; ownComp = false; }
    //grands the ownership of 'param', Component will be freed by ~TcapEntity()
    void        ownParam(Component* p)  { param = p; ownComp = true; }

    //throws CustomException
    void encode(RawBuffer& operation, RawBuffer& params) const throw(CustomException)
    {
        operation.clear(); params.clear();
        operation.push_back(opcode);
        if (param)
            param->encode(params);
    }

protected:
    uint8_t     id;
    TCEntityKind ekind;
    uint8_t     opcode;
    Component*	param;
    bool        ownComp; //this Entity is owner of 'param'
};

class TCResult : public TcapEntity {
public:
    TCResult(uint8_t tId, uint8_t tOpCode)
        : TcapEntity(tId, TcapEntity::tceResult, tOpCode)
    { }
};

class TCResultNL : public TcapEntity {
public:
    TCResultNL(uint8_t tId, uint8_t tOpCode)
        : TcapEntity(tId, TcapEntity::tceResultNL, tOpCode)
    { }
};

class TCError : public TcapEntity {
public:
    TCError(uint8_t tId, uint8_t tOpCode)
        : TcapEntity(tId, TcapEntity::tceError, tOpCode)
    { }
};

} //inap
} //inman
} //smsc
#endif /* __SMSC_INMAN_TCAP_ENTITY__ */

