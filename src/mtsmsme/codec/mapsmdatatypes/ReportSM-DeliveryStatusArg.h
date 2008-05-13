#ifndef	_ReportSM_DeliveryStatusArg_H_
#define	_ReportSM_DeliveryStatusArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include "AddressString.h"
#include "SM-DeliveryOutcome.h"
#include "AbsentSubscriberDiagnosticSM.h"
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ReportSM-DeliveryStatusArg */
typedef struct ReportSM_DeliveryStatusArg {
	ISDN_AddressString_t	 msisdn;
	AddressString_t	 serviceCentreAddress;
	SM_DeliveryOutcome_t	 sm_DeliveryOutcome;
	AbsentSubscriberDiagnosticSM_t	*absentSubscriberDiagnosticSM	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*gprsSupportIndicator	/* OPTIONAL */;
	NULL_t	*deliveryOutcomeIndicator	/* OPTIONAL */;
	SM_DeliveryOutcome_t	*additionalSM_DeliveryOutcome	/* OPTIONAL */;
	AbsentSubscriberDiagnosticSM_t	*additionalAbsentSubscriberDiagnosticSM	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ReportSM_DeliveryStatusArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ReportSM_DeliveryStatusArg;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _ReportSM_DeliveryStatusArg_H_ */
