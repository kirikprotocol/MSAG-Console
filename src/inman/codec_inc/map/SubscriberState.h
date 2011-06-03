#ifndef	_SubscriberState_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SubscriberState_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include <NotReachableReason.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SubscriberState_PR {
	SubscriberState_PR_NOTHING,	/* No components present */
	SubscriberState_PR_assumedIdle,
	SubscriberState_PR_camelBusy,
	SubscriberState_PR_netDetNotReachable,
	SubscriberState_PR_notProvidedFromVLR
} SubscriberState_PR;

/* SubscriberState */
typedef struct SubscriberState {
	SubscriberState_PR present;
	union SubscriberState_u {
		NULL_t	 assumedIdle;
		NULL_t	 camelBusy;
		NotReachableReason_t	 netDetNotReachable;
		NULL_t	 notProvidedFromVLR;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SubscriberState_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SubscriberState;

#ifdef __cplusplus
}
#endif

#endif	/* _SubscriberState_H_ */
