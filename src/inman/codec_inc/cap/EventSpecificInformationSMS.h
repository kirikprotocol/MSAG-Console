#ident "$Id$"

#ifndef	_EventSpecificInformationSMS_H_
#define	_EventSpecificInformationSMS_H_


#include <asn_application.h>

/* Including external dependencies */
#include <MO-SMSCause.h>
#include <constr_SEQUENCE.h>
#include <MT-SMSCause.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum EventSpecificInformationSMS_PR {
	EventSpecificInformationSMS_PR_NOTHING,	/* No components present */
	EventSpecificInformationSMS_PR_o_smsFailureSpecificInfo,
	EventSpecificInformationSMS_PR_o_smsSubmissionSpecificInfo,
	EventSpecificInformationSMS_PR_t_smsFailureSpecificInfo,
	EventSpecificInformationSMS_PR_t_smsDeliverySpecificInfo
} EventSpecificInformationSMS_PR;

/* EventSpecificInformationSMS */
typedef struct EventSpecificInformationSMS {
	EventSpecificInformationSMS_PR present;
	union EventSpecificInformationSMS_u {
		struct o_smsFailureSpecificInfo_s {
			MO_SMSCause_t	*failureCause	/* OPTIONAL */;
			/*
			 * This type is extensible,
			 * possible extensions are below.
			 */
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} o_smsFailureSpecificInfo;
		struct o_smsSubmissionSpecificInfo_s {
			/*
			 * This type is extensible,
			 * possible extensions are below.
			 */
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} o_smsSubmissionSpecificInfo;
		struct t_smsFailureSpecificInfo_s {
			MT_SMSCause_t	*failureCause	/* OPTIONAL */;
			/*
			 * This type is extensible,
			 * possible extensions are below.
			 */
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} t_smsFailureSpecificInfo;
		struct t_smsDeliverySpecificInfo_s {
			/*
			 * This type is extensible,
			 * possible extensions are below.
			 */
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} t_smsDeliverySpecificInfo;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EventSpecificInformationSMS_t;

typedef EventSpecificInformationSMS::EventSpecificInformationSMS_u::o_smsFailureSpecificInfo_s
  o_smsFailureSpecificInfo_t;
typedef EventSpecificInformationSMS::EventSpecificInformationSMS_u::o_smsSubmissionSpecificInfo_s
  o_smsSubmissionSpecificInfo_t;
typedef EventSpecificInformationSMS::EventSpecificInformationSMS_u::t_smsFailureSpecificInfo_s
  t_smsFailureSpecificInfo_t;
typedef EventSpecificInformationSMS::EventSpecificInformationSMS_u::t_smsDeliverySpecificInfo_s
  t_smsDeliverySpecificInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EventSpecificInformationSMS;

#ifdef __cplusplus
}
#endif

#endif	/* _EventSpecificInformationSMS_H_ */
