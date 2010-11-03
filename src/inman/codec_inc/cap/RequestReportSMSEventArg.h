#ident "$Id$"

#ifndef	_RequestReportSMSEventArg_H_
#define	_RequestReportSMSEventArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Extensions;
struct SMSEvent;

typedef struct sMSEvents_s {
	A_SEQUENCE_OF(struct SMSEvent) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} sMSEvents_t;

/* RequestReportSMSEventArg */
typedef struct RequestReportSMSEventArg {
	struct sMSEvents_s 	sMSEvents;
	struct Extensions	*extensions	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RequestReportSMSEventArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RequestReportSMSEventArg;

/* Referred external types */
#include <Extensions.h>
#include <SMSEvent.h>

#ifdef __cplusplus
}
#endif

#endif	/* _RequestReportSMSEventArg_H_ */
