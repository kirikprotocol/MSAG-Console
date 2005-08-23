#ifndef	_ResetTimerSMSArg_H_
#define	_ResetTimerSMSArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <TimerID.h>
#include <TimerValue.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Extensions;

/* ResetTimerSMSArg */
typedef struct ResetTimerSMSArg {
	TimerID_t	*timerID	/* DEFAULT 0 */;
	TimerValue_t	 timervalue;
	struct Extensions	*extensions	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ResetTimerSMSArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ResetTimerSMSArg;

/* Referred external types */
#include <Extensions.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ResetTimerSMSArg_H_ */
