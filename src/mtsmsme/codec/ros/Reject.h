#ifndef	_Reject_H_
#define	_Reject_H_


#include <asn_application.h>

/* Including external dependencies */
#include "InvokeId.h"
#include "GeneralProblem.h"
#include "InvokeProblem.h"
#include "ReturnResultProblem.h"
#include "ReturnErrorProblem.h"
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum problem_PR {
	problem_PR_NOTHING,	/* No components present */
	problem_PR_general,
	problem_PR_invoke,
	problem_PR_returnResult,
	problem_PR_returnError
} problem_PR;

/* Reject */
typedef struct Reject {
	InvokeId_t	 invokeId;
	struct problem {
		problem_PR present;
		union Reject__problem_u {
			GeneralProblem_t	 general;
			InvokeProblem_t	 invoke;
			ReturnResultProblem_t	 returnResult;
			ReturnErrorProblem_t	 returnError;
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
