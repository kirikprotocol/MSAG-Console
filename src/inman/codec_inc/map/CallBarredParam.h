#ifndef	_CallBarredParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CallBarredParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <CallBarringCause.h>
#include <ExtensibleCallBarredParam.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CallBarredParam_PR {
	CallBarredParam_PR_NOTHING,	/* No components present */
	CallBarredParam_PR_callBarringCause,
	CallBarredParam_PR_extensibleCallBarredParam,
} CallBarredParam_PR;

/* CallBarredParam */
typedef struct CallBarredParam {
	CallBarredParam_PR present;
	union CallBarredParam_u {
		CallBarringCause_t	 callBarringCause;
		ExtensibleCallBarredParam_t	 extensibleCallBarredParam;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CallBarredParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CallBarredParam;

#ifdef __cplusplus
}
#endif

#endif	/* _CallBarredParam_H_ */
