#ifndef	_EventReportSMSArg_H_
#define	_EventReportSMSArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <EventTypeSMS.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct EventSpecificInformationSMS;
struct MiscCallInfo;
struct Extensions;

/* EventReportSMSArg */
typedef struct EventReportSMSArg {
	EventTypeSMS_t	 eventTypeSMS;
	struct EventSpecificInformationSMS	*eventSpecificInformationSMS	/* OPTIONAL */;
	struct MiscCallInfo	*miscCallInfo	/* DEFAULT {messageType request} */;
	struct Extensions	*extensions	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EventReportSMSArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EventReportSMSArg;

/* Referred external types */
#include <EventSpecificInformationSMS.h>
#include <MiscCallInfo.h>
#include <Extensions.h>

#ifdef __cplusplus
}
#endif

#endif	/* _EventReportSMSArg_H_ */
