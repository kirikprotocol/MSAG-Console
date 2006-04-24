#ident "$Id$"

#ifndef	_Ext_ForwFeatureList_H_
#define	_Ext_ForwFeatureList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Ext_ForwFeature;

/* Ext-ForwFeatureList */
typedef struct Ext_ForwFeatureList {
	A_SEQUENCE_OF(struct Ext_ForwFeature) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Ext_ForwFeatureList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_ForwFeatureList;

/* Referred external types */
#include <Ext-ForwFeature.h>

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_ForwFeatureList_H_ */
