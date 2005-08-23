#include <asn_internal.h>

#include "ResetTimerSMSArg.h"

static asn_TYPE_member_t asn_MBR_ResetTimerSMSArg_1[] = {
	{ ATF_POINTER, 1, offsetof(struct ResetTimerSMSArg, timerID),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		(void *)&asn_DEF_TimerID,
		0,	/* Defer constraints checking to the member type */
		"timerID"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ResetTimerSMSArg, timervalue),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		(void *)&asn_DEF_TimerValue,
		0,	/* Defer constraints checking to the member type */
		"timervalue"
		},
	{ ATF_POINTER, 1, offsetof(struct ResetTimerSMSArg, extensions),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		(void *)&asn_DEF_Extensions,
		0,	/* Defer constraints checking to the member type */
		"extensions"
		},
};
static ber_tlv_tag_t asn_DEF_ResetTimerSMSArg_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ResetTimerSMSArg_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* timerID at 1132 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* timervalue at 1133 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* extensions at 1134 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ResetTimerSMSArg_1_specs = {
	sizeof(struct ResetTimerSMSArg),
	offsetof(struct ResetTimerSMSArg, _asn_ctx),
	asn_MAP_ResetTimerSMSArg_1_tag2el,
	3,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ResetTimerSMSArg = {
	"ResetTimerSMSArg",
	"ResetTimerSMSArg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ResetTimerSMSArg_1_tags,
	sizeof(asn_DEF_ResetTimerSMSArg_1_tags)
		/sizeof(asn_DEF_ResetTimerSMSArg_1_tags[0]), /* 1 */
	asn_DEF_ResetTimerSMSArg_1_tags,	/* Same as above */
	sizeof(asn_DEF_ResetTimerSMSArg_1_tags)
		/sizeof(asn_DEF_ResetTimerSMSArg_1_tags[0]), /* 1 */
	asn_MBR_ResetTimerSMSArg_1,
	3,	/* Elements count */
	&asn_SPC_ResetTimerSMSArg_1_specs	/* Additional specs */
};

