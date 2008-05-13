#ifndef	_InformServiceCentreArg_H_
#define	_InformServiceCentreArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include "MW-Status.h"
#include "AbsentSubscriberDiagnosticSM.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* InformServiceCentreArg */
typedef struct InformServiceCentreArg {
	ISDN_AddressString_t	*storedMSISDN	/* OPTIONAL */;
	MW_Status_t	*mw_Status	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	AbsentSubscriberDiagnosticSM_t	*absentSubscriberDiagnosticSM	/* OPTIONAL */;
	AbsentSubscriberDiagnosticSM_t	*additionalAbsentSubscriberDiagnosticSM	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} InformServiceCentreArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InformServiceCentreArg;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _InformServiceCentreArg_H_ */
