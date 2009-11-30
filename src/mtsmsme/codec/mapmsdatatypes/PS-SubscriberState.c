#include <asn_internal.h>

#include "PS-SubscriberState.h"

static asn_TYPE_member_t asn_MBR_PS_SubscriberState_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.notProvidedFromSGSN),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"notProvidedFromSGSN"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_Detached),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ps-Detached"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_AttachedNotReachableForPaging),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ps-AttachedNotReachableForPaging"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_AttachedReachableForPaging),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ps-AttachedReachableForPaging"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_PDP_ActiveNotReachableForPaging),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PDP_ContextInfoList,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ps-PDP-ActiveNotReachableForPaging"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.ps_PDP_ActiveReachableForPaging),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PDP_ContextInfoList,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ps-PDP-ActiveReachableForPaging"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PS_SubscriberState, choice.netDetNotReachable),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_NotReachableReason,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"netDetNotReachable"
		},
};
static asn_TYPE_tag2member_t asn_MAP_PS_SubscriberState_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 6, 0, 0 }, /* netDetNotReachable at 556 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* notProvidedFromSGSN at 550 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* ps-Detached at 551 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* ps-AttachedNotReachableForPaging at 552 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* ps-AttachedReachableForPaging at 553 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* ps-PDP-ActiveNotReachableForPaging at 554 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* ps-PDP-ActiveReachableForPaging at 555 */
};
static asn_CHOICE_specifics_t asn_SPC_PS_SubscriberState_specs_1 = {
	sizeof(struct PS_SubscriberState),
	offsetof(struct PS_SubscriberState, _asn_ctx),
	offsetof(struct PS_SubscriberState, present),
	sizeof(((struct PS_SubscriberState *)0)->present),
	asn_MAP_PS_SubscriberState_tag2el_1,
	7,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_PS_SubscriberState = {
	"PS-SubscriberState",
	"PS-SubscriberState",
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
	asn_MBR_PS_SubscriberState_1,
	7,	/* Elements count */
	&asn_SPC_PS_SubscriberState_specs_1	/* Additional specs */
};

