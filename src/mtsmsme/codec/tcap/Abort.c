#include <asn_internal.h>

#include "Abort.h"

static asn_TYPE_member_t asn_MBR_reason_3[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct reason, choice.p_abortCause),
		(ASN_TAG_CLASS_APPLICATION | (10 << 2)),
		0,
		&asn_DEF_P_AbortCause,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"p-abortCause"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct reason, choice.u_abortCause),
		(ASN_TAG_CLASS_APPLICATION | (11 << 2)),
		0,
		&asn_DEF_DialoguePortion,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"u-abortCause"
		},
};
static asn_TYPE_tag2member_t asn_MAP_reason_tag2el_3[] = {
    { (ASN_TAG_CLASS_APPLICATION | (10 << 2)), 0, 0, 0 }, /* p-abortCause at 54 */
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 1, 0, 0 } /* u-abortCause at 55 */
};
static asn_CHOICE_specifics_t asn_SPC_reason_specs_3 = {
	sizeof(struct reason),
	offsetof(struct reason, _asn_ctx),
	offsetof(struct reason, present),
	sizeof(((struct reason *)0)->present),
	asn_MAP_reason_tag2el_3,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_reason_3 = {
	"reason",
	"reason",
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
	asn_MBR_reason_3,
	2,	/* Elements count */
	&asn_SPC_reason_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_Abort_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Abort, dtid),
		(ASN_TAG_CLASS_APPLICATION | (9 << 2)),
		0,
		&asn_DEF_DestTransactionID,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dtid"
		},
	{ ATF_POINTER, 1, offsetof(struct Abort, reason),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_reason_3,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"reason"
		},
};
static ber_tlv_tag_t asn_DEF_Abort_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Abort_tag2el_1[] = {
    { (ASN_TAG_CLASS_APPLICATION | (9 << 2)), 0, 0, 0 }, /* dtid at 53 */
    { (ASN_TAG_CLASS_APPLICATION | (10 << 2)), 1, 0, 0 }, /* p-abortCause at 54 */
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 1, 0, 0 } /* u-abortCause at 55 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Abort_specs_1 = {
	sizeof(struct Abort),
	offsetof(struct Abort, _asn_ctx),
	asn_MAP_Abort_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Abort = {
	"Abort",
	"Abort",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Abort_tags_1,
	sizeof(asn_DEF_Abort_tags_1)
		/sizeof(asn_DEF_Abort_tags_1[0]), /* 1 */
	asn_DEF_Abort_tags_1,	/* Same as above */
	sizeof(asn_DEF_Abort_tags_1)
		/sizeof(asn_DEF_Abort_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_Abort_1,
	2,	/* Elements count */
	&asn_SPC_Abort_specs_1	/* Additional specs */
};

