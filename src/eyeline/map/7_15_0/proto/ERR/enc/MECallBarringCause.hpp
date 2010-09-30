#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MECALLBARRINGCAUSE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MECALLBARRINGCAUSE_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeENUM.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
 CallBarringCause ::= ENUMERATED {
        barringServiceActive  (0),
        operatorBarring  (1)}
*/
typedef asn1::ber::EncoderOfENUM MECallBarringCause;

}}}}

#endif
