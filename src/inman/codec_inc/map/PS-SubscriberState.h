#ifndef	_PS_SubscriberState_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_PS_SubscriberState_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include <PDP-ContextInfoList.h>
#include <NotReachableReason.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum PS_SubscriberState_PR {
	PS_SubscriberState_PR_NOTHING,	/* No components present */
	PS_SubscriberState_PR_notProvidedFromSGSN,
	PS_SubscriberState_PR_ps_Detached,
	PS_SubscriberState_PR_ps_AttachedNotReachableForPaging,
	PS_SubscriberState_PR_ps_AttachedReachableForPaging,
	PS_SubscriberState_PR_ps_PDP_ActiveNotReachableForPaging,
	PS_SubscriberState_PR_ps_PDP_ActiveReachableForPaging,
	PS_SubscriberState_PR_netDetNotReachable
} PS_SubscriberState_PR;

/* PS-SubscriberState */
typedef struct PS_SubscriberState {
	PS_SubscriberState_PR present;
	union PS_SubscriberState_u {
		NULL_t	 notProvidedFromSGSN;
		NULL_t	 ps_Detached;
		NULL_t	 ps_AttachedNotReachableForPaging;
		NULL_t	 ps_AttachedReachableForPaging;
		PDP_ContextInfoList_t	 ps_PDP_ActiveNotReachableForPaging;
		PDP_ContextInfoList_t	 ps_PDP_ActiveReachableForPaging;
		NotReachableReason_t	 netDetNotReachable;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PS_SubscriberState_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PS_SubscriberState;

#ifdef __cplusplus
}
#endif

#endif	/* _PS_SubscriberState_H_ */
