#ifndef	_Undirectional_H_
#define	_Undirectional_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ComponentPortion.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct EXT;

/* Undirectional */
typedef struct Undirectional {
	struct EXT	*dialoguePortion	/* OPTIONAL */;
	ComponentPortion_t	 components;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Undirectional_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Undirectional;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "DialoguePortion.h"

#endif	/* _Undirectional_H_ */
