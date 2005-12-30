#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_ENTITY__
#define __SMSC_INMAN_INAP_ENTITY__

#include "inman/common/types.hpp"
#include "inman/comp/compdefs.hpp"

using smsc::inman::comp::Component;

namespace smsc  {
namespace inman {
namespace inap  {

typedef std::vector<unsigned char> RawBuffer;

class TcapEntity
{
protected:
    UCHAR_T     id;
    UCHAR_T     opcode;
    UCHAR_T     tag;
    Component*	param;
    bool        ownComp; //this Entity is owner of 'param'

public:
    TcapEntity(UCHAR_T tId, UCHAR_T tTag = 0, UCHAR_T tOpCode = 0)
        : id(tId), tag(tTag), opcode(tOpCode), param(NULL), ownComp(false) { }

    virtual ~TcapEntity() { if (ownComp) delete param; }

    USHORT_T    getId() const           { return id; }
    UCHAR_T     getTag() const          { return tag; }
    void        setTag(UCHAR_T t)       { tag = t; }
    UCHAR_T     getOpcode() const       { return opcode; }
    void        setOpcode(UCHAR_T opc)  { opcode = opc; }
    Component*  getParam() const        { return param; }
    //sets 'param' without passing ownership, it's caller responsibility to free Component
    void        setParam(Component* p)     { param = p; ownComp = false; }
    //grands the ownership of 'param', Component will be freed by ~TcapEntity()
    void        ownParam(Component* p)     { param = p; ownComp = true; }

    //throws CustomException
    void encode(RawBuffer& operation, RawBuffer& params) throw(CustomException)
    {
        operation.clear(); params.clear();
        operation.push_back(opcode);
        if (param)
            param->encode(params);
    }

//abstract:
    virtual void send() throw(CustomException) = 0;
};

} //inap
} //inman
} //smsc
#endif /* __SMSC_INMAN_INAP_ENTITY__ */

