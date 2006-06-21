#ident "$Id$"

#include <asn_internal.h>

#include "SubscriberState.h"

static asn_TYPE_member_t asn_MBR_SubscriberState_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SubscriberState, choice.assumedIdle),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"assumedIdle"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SubscriberState, choice.camelBusy),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"camelBusy"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SubscriberState, choice.netDetNotReachable),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_NotReachableReason,
		0,	/* Defer constraints checking to the member type */
		"netDetNotReachable"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SubscriberState, choice.notProvidedFromVLR),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"notProvidedFromVLR"
		},
};
static asn_TYPE_tag2member_t asn_MAP_SubscriberState_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 2, 0, 0 }, /* netDetNotReachable at 503 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* assumedIdle at 501 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* camelBusy at 502 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 3, 0, 0 } /* notProvidedFromVLR at 504 */
};
static asn_CHOICE_specifics_t asn_SPC_SubscriberState_1_specs = {
	sizeof(struct SubscriberState),
	offsetof(struct SubscriberState, _asn_ctx),
	offsetof(struct SubscriberState, present),
	sizeof(((struct SubscriberState *)0)->present),
	asn_MAP_SubscriberState_1_tag2el,
	4,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_SubscriberState = {
	"SubscriberState",
	"SubscriberState",
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
	asn_MBR_SubscriberState_1,
	4,	/* Elements count */
	&asn_SPC_SubscriberState_1_specs	/* Additional specs */
};

