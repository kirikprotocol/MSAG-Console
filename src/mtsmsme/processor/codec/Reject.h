#ifndef	_Reject_H_
#define	_Reject_H_


#include <asn_application.h>

/* Including external dependencies */
#include "InvokeIdType.h"
#include <NULL.h>
#include <constr_CHOICE.h>
#include "GeneralProblem.h"
#include "InvokeProblem.h"
#include "ReturnResultProblem.h"
#include "ReturnErrorProblem.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum invokeID_PR {
	invokeID_PR_NOTHING,	/* No components present */
	invokeID_PR_derivable,
	invokeID_PR_not_derivable
} invokeID_PR;
typedef enum problem_PR {
	problem_PR_NOTHING,	/* No components present */
	problem_PR_generalProblem,
	problem_PR_invokeProblem,
	problem_PR_returnResultProblem,
	problem_PR_returnErrorProblem
} problem_PR;

/* Reject */
typedef struct Reject {
	struct invokeID {
		invokeID_PR present;
		union Reject__invokeID_u {
			InvokeIdType_t	 derivable;
			NULL_t	 not_derivable;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} invokeID;
	struct problem {
		problem_PR present;
		union Reject__problem_u {
			GeneralProblem_t	 generalProblem;
			InvokeProblem_t	 invokeProblem;
			ReturnResultProblem_t	 returnResultProblem;
			ReturnErrorProblem_t	 returnErrorProblem;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} problem;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Reject_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Reject;

#ifdef __cplusplus
}
#endif

#endif	/* _Reject_H_ */
