#ifndef	_ComponentPortion_H_
#define	_ComponentPortion_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Component;

/* ComponentPortion */
typedef struct ComponentPortion {
	A_SEQUENCE_OF(struct Component) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ComponentPortion_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ComponentPortion;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "Component.h"

#endif	/* _ComponentPortion_H_ */
