#ifndef	_GPRSMSClass_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_GPRSMSClass_H_


#include <asn_application.h>

/* Including external dependencies */
#include <MSNetworkCapability.h>
#include <MSRadioAccessCapability.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPRSMSClass */
typedef struct GPRSMSClass {
	MSNetworkCapability_t	 mSNetworkCapability;
	MSRadioAccessCapability_t	*mSRadioAccessCapability	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GPRSMSClass_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GPRSMSClass;

#ifdef __cplusplus
}
#endif

#endif	/* _GPRSMSClass_H_ */
