#ifndef	_Associate_source_diagnostic_H_
#define	_Associate_source_diagnostic_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Associate_source_diagnostic_PR {
	Associate_source_diagnostic_PR_NOTHING,	/* No components present */
	Associate_source_diagnostic_PR_dialogue_service_user,
	Associate_source_diagnostic_PR_dialogue_service_provider
} Associate_source_diagnostic_PR;
typedef enum dialogue_service_user {
	dialogue_service_user_null	= 0,
	dialogue_service_user_no_reason_given	= 1,
	dialogue_service_user_application_context_name_not_supported	= 2
} e_dialogue_service_user;
typedef enum dialogue_service_provider {
	dialogue_service_provider_null	= 0,
	dialogue_service_provider_no_reason_given	= 1,
	dialogue_service_provider_no_common_dialogue_portion	= 2
} e_dialogue_service_provider;

/* Associate-source-diagnostic */
typedef struct Associate_source_diagnostic {
	Associate_source_diagnostic_PR present;
	union Associate_source_diagnostic_u {
		long	 dialogue_service_user;
		long	 dialogue_service_provider;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Associate_source_diagnostic_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Associate_source_diagnostic;

#ifdef __cplusplus
}
#endif

#endif	/* _Associate_source_diagnostic_H_ */
