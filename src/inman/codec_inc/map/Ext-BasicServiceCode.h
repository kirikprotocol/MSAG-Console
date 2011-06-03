#ifndef	_Ext_BasicServiceCode_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_Ext_BasicServiceCode_H_


#include <asn_application.h>

/* Including external dependencies */
#include <Ext-BearerServiceCode.h>
#include <Ext-TeleserviceCode.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Ext_BasicServiceCode_PR {
	Ext_BasicServiceCode_PR_NOTHING,	/* No components present */
	Ext_BasicServiceCode_PR_ext_BearerService,
	Ext_BasicServiceCode_PR_ext_Teleservice
} Ext_BasicServiceCode_PR;

/* Ext-BasicServiceCode */
typedef struct Ext_BasicServiceCode {
	Ext_BasicServiceCode_PR present;
	union Ext_BasicServiceCode_u {
		Ext_BearerServiceCode_t	 ext_BearerService;
		Ext_TeleserviceCode_t	 ext_Teleservice;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Ext_BasicServiceCode_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_BasicServiceCode;

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_BasicServiceCode_H_ */
