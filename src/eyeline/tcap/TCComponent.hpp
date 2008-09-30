/* ************************************************************************** *
 * Base class implementing TCAP messages component (ROS component) definition.
 * ************************************************************************** */
#ifndef __TC_COMPONENT_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_COMPONENT_DEFS_HPP

#include "eyeline/asn1/ASNTypes.hpp"

namespace eyelinecom {
namespace tcap {

using eyelinecom::asn1::ASTag;
using eyelinecom::asn1::ASTypeAC;
//using eyelinecom::asn1::AbstractSyntax;
using eyelinecom::asn1::ASTypeTagging;
using eyelinecom::asn1::EncodedOID;
using eyelinecom::asn1::BITBuffer;

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
    const EncodedOID &  _appCtx;

    ROSComponentAC(Kind ros_kind, uint8_t op_code, const EncodedOID & app_ctx)
        : _kind(ros_kind), _opCode(op_code), _appCtx(app_ctx)
    { }

    // ---------------------------------
    // ASTypeAC interface methods
    // ---------------------------------

    //REQ: presentation > valNone, if use_rule == valRule, otherwise presentation == valDecoded
    ENCResult Encode(BITBuffer & buf, EncodingRule use_rule) /*throw ASN1CodecError*/;
    //REQ: presentation == valEncoded | valMixed (setEncoding was called)
    //OUT: type presentation = valDecoded, components (if exist) presentation = valDecoded,
    //in case of decMoreInput, stores decoding context
    DECResult Decode(void) /*throw ASN1CodecError*/;
    //REQ: presentation == valEncoded (setEncoding was called)
    //OUT: type presentation = valMixed | valDecoded, 
    //     deferred components presentation = valEncoded
    //NOTE: if num_tags == 0, all components decoding is deferred 
    //in case of decMoreInput, stores decoding context 
    DECResult Demux(uint16_t num_tags = 0,
                    const ASTypeTagging (* defer_tag)[] = NULL) /*throw ASN1CodecError*/;
};

} //tcap
} //eyelinecom

#endif /* __TC_COMPONENT_DEFS_HPP */

