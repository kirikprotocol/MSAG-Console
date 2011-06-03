#ifndef	_SystemFailureParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SystemFailureParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NetworkResource.h>
#include <ExtensibleSystemFailureParam.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SystemFailureParam_PR {
	SystemFailureParam_PR_NOTHING,	/* No components present */
	SystemFailureParam_PR_networkResource,
	SystemFailureParam_PR_extensibleSystemFailureParam,
} SystemFailureParam_PR;

/* SystemFailureParam */
typedef struct SystemFailureParam {
	SystemFailureParam_PR present;
	union SystemFailureParam_u {
		NetworkResource_t	 networkResource;
		ExtensibleSystemFailureParam_t	 extensibleSystemFailureParam;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SystemFailureParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SystemFailureParam;

#ifdef __cplusplus
}
#endif

#endif	/* _SystemFailureParam_H_ */
