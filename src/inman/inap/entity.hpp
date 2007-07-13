#ident "$Id$"
/* ************************************************************************* *
 * Transaction Component with local operation tag (no objId as opcode).
 * ************************************************************************* */
#ifndef __SMSC_INMAN_TCAP_ENTITY__
#define __SMSC_INMAN_TCAP_ENTITY__

#include "inman/common/types.hpp"
#include "inman/comp/compdefs.hpp"

using smsc::inman::comp::Component;

namespace smsc  {
namespace inman {
namespace inap  {

typedef std::vector<unsigned char> RawBuffer;

//Transaction Component
class TcapEntity {
public:
    typedef enum {
        tceNone = 0, tceInvoke, tceResult, tceResultNL, tceError
    } TCEntityKind;

    TcapEntity(UCHAR_T tId, TCEntityKind e_kind = tceNone, UCHAR_T tOpCode = 0)
        : id(tId), ekind(e_kind), opcode(tOpCode), param(NULL), ownComp(false) { }

    virtual ~TcapEntity() { if (ownComp) delete param; }

    inline TCEntityKind kind() const           { return ekind; }
    inline UCHAR_T     getId() const           { return id; }
    inline UCHAR_T     getOpcode() const       { return opcode; }
    inline Component*  getParam() const        { return param; }
    //sets 'param' without passing ownership, it's caller responsibility to free Component
    inline void        setParam(Component* p)  { param = p; ownComp = false; }
    //grands the ownership of 'param', Component will be freed by ~TcapEntity()
    inline void        ownParam(Component* p)  { param = p; ownComp = true; }

    //throws CustomException
    void encode(RawBuffer& operation, RawBuffer& params) throw(CustomException)
    {
        operation.clear(); params.clear();
        operation.push_back(opcode);
        if (param)
            param->encode(params);
    }

protected:
    UCHAR_T     id;
    TCEntityKind ekind;
    UCHAR_T     opcode;
    Component*	param;
    bool        ownComp; //this Entity is owner of 'param'
};

class TCResult : public TcapEntity {
public:
    TCResult(UCHAR_T tId, UCHAR_T tOpCode)
        : TcapEntity(tId, TcapEntity::tceResult, tOpCode)
    { }
};

class TCResultNL : public TcapEntity {
public:
    TCResultNL(UCHAR_T tId, UCHAR_T tOpCode)
        : TcapEntity(tId, TcapEntity::tceResultNL, tOpCode)
    { }
};

class TCError : public TcapEntity {
public:
    TCError(UCHAR_T tId, UCHAR_T tOpCode)
        : TcapEntity(tId, TcapEntity::tceError, tOpCode)
    { }
};

} //inap
} //inman
} //smsc
#endif /* __SMSC_INMAN_TCAP_ENTITY__ */

