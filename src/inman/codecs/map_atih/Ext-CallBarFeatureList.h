#ident "$Id$"

#ifndef	_Ext_CallBarFeatureList_H_
#define	_Ext_CallBarFeatureList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Ext_CallBarringFeature;

/* Ext-CallBarFeatureList */
typedef struct Ext_CallBarFeatureList {
	A_SEQUENCE_OF(struct Ext_CallBarringFeature) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Ext_CallBarFeatureList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_CallBarFeatureList;

/* Referred external types */
#include <Ext-CallBarringFeature.h>

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_CallBarFeatureList_H_ */
