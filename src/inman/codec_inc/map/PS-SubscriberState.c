#ident "$Id$"

#include <asn_internal.h>

#include "PS-SubscriberState.h"

static asn_TYPE_member_t asn_MBR_PS_SubscriberState_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.notProvidedFromSGSN),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"notProvidedFromSGSN"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_Detached),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"ps-Detached"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_AttachedNotReachableForPaging),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"ps-AttachedNotReachableForPaging"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_AttachedReachableForPaging),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"ps-AttachedReachableForPaging"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_PDP_ActiveNotReachableForPaging),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PDP_ContextInfoList,
		0,	/* Defer constraints checking to the member type */
		"ps-PDP-ActiveNotReachableForPaging"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_PDP_ActiveReachableForPaging),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PDP_ContextInfoList,
		0,	/* Defer constraints checking to the member type */
		"ps-PDP-ActiveReachableForPaging"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.netDetNotReachable),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_NotReachableReason,
		0,	/* Defer constraints checking to the member type */
		"netDetNotReachable"
		},
};
static asn_TYPE_tag2member_t asn_MAP_PS_SubscriberState_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 6, 0, 0 }, /* netDetNotReachable at 515 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* notProvidedFromSGSN at 508 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* ps-Detached at 509 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* ps-AttachedNotReachableForPaging at 510 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* ps-AttachedReachableForPaging at 511 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* ps-PDP-ActiveNotReachableForPaging at 512 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* ps-PDP-ActiveReachableForPaging at 513 */
};
static asn_CHOICE_specifics_t asn_SPC_PS_SubscriberState_1_specs = {
	sizeof(struct PS_SubscriberState),
	offsetof(struct PS_SubscriberState, _asn_ctx),
	offsetof(struct PS_SubscriberState, present),
	sizeof(((struct PS_SubscriberState *)0)->present),
	asn_MAP_PS_SubscriberState_1_tag2el,
	7,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_PS_SubscriberState = {
	"PS-SubscriberState",
	"PS-SubscriberState",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
#ifndef ASN1_XER_NOT_USED
	CHOICE_decode_xer,
	CHOICE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	asn_MBR_PS_SubscriberState_1,
	7,	/* Elements count */
	&asn_SPC_PS_SubscriberState_1_specs	/* Additional specs */
};

