/* ************************************************************************** *
 * Various helper functions for use in ROS component encoding/decoding.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INAP_COMPS_UTL_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_COMPS_UTL_HPP__

#include "logger/Logger.h"

#include "util/TonNpiAddress.hpp"

#include "inman/comp/compdefs.hpp"
#include "inman/asn1rt/constr_TYPE.h"
#include "inman/asn1rt/OCTET_STRING.h"

extern "C" int print2vec(const void *buffer, size_t size, void *app_key);

#define ASNCODEC_LOG_ENC(er, asnDef, compId)	if (er.encoded == -1) { \
	smsc_log_error(compLogger, \
	"%s: ASN1 Encoding of %s failed at %s", compId, asnDef.name, er.failed_type->name); \
	throw ASN1EncodeError(asnDef.name, er.failed_type->name, compId); }

#define ASNCODEC_LOG_DEC(dcmd, drc, asnDef, compId)  if (drc.code != RC_OK) { \
	smsc_log_error(compLogger, \
	"%s: ASN1 Decoding of %s failed with code %s at byte: %d\n", compId, asnDef.name, \
	(drc.code == RC_FAIL) ? "RC_FAIL" : "RC_WMORE", drc.consumed); \
        asnDef.free_struct(&asnDef, dcmd, 0); \
	throw ASN1DecodeError(asnDef.name, drc.code, drc.consumed, compId); }

namespace smsc {
namespace inman {
namespace comp {

using smsc::logger::Logger;
using smsc::util::TonNpiAddress;

using smsc::inman::comp::ASN1DecodeError;
using smsc::inman::comp::ASN1EncodeError;


extern std::string printType2String(asn_TYPE_descriptor_t * def, void * tStruct);

inline void smsc_log_component(Logger *lgi, asn_TYPE_descriptor_t * asnDef, void * tStruct)
{
    if (lgi->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG)) {
	std::string sdump = printType2String(asnDef, tStruct);
	lgi->log_(smsc::logger::Logger::LEVEL_DEBUG, sdump);
    }
}

/*
 * Returns number of signals in address, zero in case of error.
 */
extern unsigned OCTET_STRING_2_Address(OCTET_STRING_t * octs, TonNpiAddress & addr);

}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_INAP_COMPS_UTL_HPP__ */
