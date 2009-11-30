#include <asn_internal.h>

#include "SubscriberState.h"

static asn_TYPE_member_t asn_MBR_SubscriberState_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SubscriberState, choice.assumedIdle),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"assumedIdle"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SubscriberState, choice.camelBusy),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"camelBusy"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SubscriberState, choice.netDetNotReachable),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_NotReachableReason,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"netDetNotReachable"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SubscriberState, choice.notProvidedFromVLR),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"notProvidedFromVLR"
		},
};
static asn_TYPE_tag2member_t asn_MAP_SubscriberState_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 2, 0, 0 }, /* netDetNotReachable at 545 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* assumedIdle at 543 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* camelBusy at 544 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 3, 0, 0 } /* notProvidedFromVLR at 546 */
};
static asn_CHOICE_specifics_t asn_SPC_SubscriberState_specs_1 = {
	sizeof(struct SubscriberState),
	offsetof(struct SubscriberState, _asn_ctx),
	offsetof(struct SubscriberState, present),
	sizeof(((struct SubscriberState *)0)->present),
	asn_MAP_SubscriberState_tag2el_1,
	4,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_SubscriberState = {
	"SubscriberState",
	"SubscriberState",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	asn_MBR_SubscriberState_1,
	4,	/* Elements count */
	&asn_SPC_SubscriberState_specs_1	/* Additional specs */
};

