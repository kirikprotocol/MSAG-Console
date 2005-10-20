#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_COMPS_UTL_HPP__
#define __SMSC_INMAN_INAP_COMPS_UTL_HPP__

#define INMAN_LOG_ON

#include "inman/common/adrutil.hpp"
#include "inman/common/cvtutil.hpp"
#include "inman/asn1rt/constr_TYPE.h"

#include "inman/asn1rt/OCTET_STRING.h"
extern "C" int print2vec(const void *buffer, size_t size, void *app_key);


#include "logger/Logger.h"
using smsc::logger::Logger;

#define ASNCODEC_LOG_ENC(er, asnDef, modName)	if (er.encoded == -1) { \
	smsc_log_error(compLogger, \
	"Encoding of %s failed at %s", asnDef.name, er.failed_type->name); \
	throw EncodeError(format(":%s: Encoding of %s failed at %s", modName, \
					asnDef.name, er.failed_type->name)); }

#define ASNCODEC_LOG_DEC(drc, asnDef, modName)	if (drc.code != RC_OK) { \
	smsc_log_error(compLogger, \
	"Decoding of %s failed with code %s at byte: %d\n", asnDef.name, \
	drc.code == RC_FAIL ? "RC_FAIL" : "RC_WMORE", drc.consumed); \
	throw DecodeError(format(":%s: Decoding of %s failed with code %s at byte: %d\n", \
	modName, asnDef.name, drc.code == RC_FAIL ? "RC_FAIL" : "RC_WMORE", drc.consumed)); }


#define INMAN_LOG_ENC(er, asnDef) ASNCODEC_LOG_ENC(er, asnDef, "InMan")

#define INMAN_LOG_DEC(drc, asnDef) ASNCODEC_LOG_DEC(drc, asnDef, "InMan")




namespace smsc {
namespace inman {
namespace comp {

extern std::string printType2String(asn_TYPE_descriptor_t * def, void * tStruct);

inline void smsc_log_component(Logger *lgi, asn_TYPE_descriptor_t * asnDef, void * tStruct)
{
    if (lgi->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG)) {
	std::string sdump = printType2String(asnDef, tStruct);
	lgi->log_(smsc::logger::Logger::LEVEL_DEBUG, sdump);
    }
}

extern Address	OCTET_STRING_2_Addres(OCTET_STRING_t * octs);


}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_INAP_COMPS_UTL_HPP__ */
