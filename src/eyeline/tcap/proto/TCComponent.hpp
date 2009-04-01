/* ************************************************************************** *
 * Base class implementing TCAP messages component (ROS component) definition.
 * ************************************************************************** */
#ifndef __TC_COMPONENT_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_COMPONENT_DEFS_HPP

#include "eyeline/asn1/ASNTypes.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

using eyeline::asn1::ASTag;
using eyeline::asn1::ASTypeAC;
using eyeline::asn1::ASTypeTagging;
using eyeline::asn1::EncodedOID;
using eyeline::asn1::BITBuffer;

//Base class for component of ROS OPERATION
class ROSComponentAC : public ASTypeAC {
public: 
    enum Kind { //ContextSpecific tag of component
        //basic components, according to
        //joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
        rosInvoke = 0x01
        , rosResult = 0x02
        , rosError = 0x03
        , rosReject = 0x04
        //additional conponent, according to
        //itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)
        , rosResultNL = 0x07
    };

    const Kind          _kind;
    const uint8_t       _opCode;
    const EncodedOID *  _appCtx;

    ROSComponentAC(Kind ros_kind, uint8_t op_code, const EncodedOID * app_ctx)
        : _kind(ros_kind), _opCode(op_code), _appCtx(app_ctx)
    { }
//    virtual ~ROSComponentAC()
//    { }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_COMPONENT_DEFS_HPP */

