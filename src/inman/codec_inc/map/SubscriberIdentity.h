#ifndef	_SubscriberIdentity_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SubscriberIdentity_H_


#include <asn_application.h>

/* Including external dependencies */
#include <IMSI.h>
#include <ISDN-AddressString.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SubscriberIdentity_PR {
	SubscriberIdentity_PR_NOTHING,	/* No components present */
	SubscriberIdentity_PR_imsi,
	SubscriberIdentity_PR_msisdn
} SubscriberIdentity_PR;

/* SubscriberIdentity */
typedef struct SubscriberIdentity {
	SubscriberIdentity_PR present;
	union SubscriberIdentity_u {
		IMSI_t	 imsi;
		ISDN_AddressString_t	 msisdn;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SubscriberIdentity_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SubscriberIdentity;

#ifdef __cplusplus
}
#endif

#endif	/* _SubscriberIdentity_H_ */
