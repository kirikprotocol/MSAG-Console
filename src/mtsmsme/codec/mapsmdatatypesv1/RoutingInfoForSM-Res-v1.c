#include <asn_internal.h>

#include "RoutingInfoForSM-Res-v1.h"

static asn_TYPE_member_t asn_MBR_sequence_4[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct sequence, locationInfo),
	  (ber_tlv_tag_t)-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_LocationInfo_v1,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"locationInfo"
		},
	{ ATF_POINTER, 1, offsetof(struct sequence, lMsId),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_LMsId_v1,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lMsId"
		},
};
static ber_tlv_tag_t asn_DEF_sequence_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_sequence_tag2el_4[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, 0, 0 }, /* lMsId at 22 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* roamingNumber at 28 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 } /* mscNumber at 29 */
};
static asn_SEQUENCE_specifics_t asn_SPC_sequence_specs_4 = {
	sizeof(struct sequence),
	offsetof(struct sequence, _asn_ctx),
	asn_MAP_sequence_tag2el_4,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_sequence_4 = {
	"sequence",
	"sequence",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_sequence_tags_4,
	sizeof(asn_DEF_sequence_tags_4)
		/sizeof(asn_DEF_sequence_tags_4[0]) - 1, /* 1 */
	asn_DEF_sequence_tags_4,	/* Same as above */
	sizeof(asn_DEF_sequence_tags_4)
		/sizeof(asn_DEF_sequence_tags_4[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_sequence_4,
	2,	/* Elements count */
	&asn_SPC_sequence_specs_4	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_choice_3[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct choice, choice.sequence),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		0,
		&asn_DEF_sequence_4,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sequence"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct choice, choice.forwardingData),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ForwardingData_v1,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"forwardingData"
		},
};
static asn_TYPE_tag2member_t asn_MAP_choice_tag2el_3[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* sequence at 21 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* forwardingData at 23 */
};
static asn_CHOICE_specifics_t asn_SPC_choice_specs_3 = {
	sizeof(struct choice),
	offsetof(struct choice, _asn_ctx),
	offsetof(struct choice, present),
	sizeof(((struct choice *)0)->present),
	asn_MAP_choice_tag2el_3,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_choice_3 = {
	"choice",
	"choice",
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
	asn_MBR_choice_3,
	2,	/* Elements count */
	&asn_SPC_choice_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_RoutingInfoForSM_Res_v1_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfoForSM_Res_v1, imsi),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfoForSM_Res_v1, choice),
		(ber_tlv_tag_t)-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_choice_3,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"choice"
		},
	{ ATF_POINTER, 1, offsetof(struct RoutingInfoForSM_Res_v1, mwd_Set),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mwd-Set"
		},
};
static ber_tlv_tag_t asn_DEF_RoutingInfoForSM_Res_v1_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_RoutingInfoForSM_Res_v1_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* imsi at 18 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* sequence at 21 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* forwardingData at 23 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* mwd-Set at 24 */
};
static asn_SEQUENCE_specifics_t asn_SPC_RoutingInfoForSM_Res_v1_specs_1 = {
	sizeof(struct RoutingInfoForSM_Res_v1),
	offsetof(struct RoutingInfoForSM_Res_v1, _asn_ctx),
	asn_MAP_RoutingInfoForSM_Res_v1_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_RoutingInfoForSM_Res_v1 = {
	"RoutingInfoForSM-Res-v1",
	"RoutingInfoForSM-Res-v1",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_RoutingInfoForSM_Res_v1_tags_1,
	sizeof(asn_DEF_RoutingInfoForSM_Res_v1_tags_1)
		/sizeof(asn_DEF_RoutingInfoForSM_Res_v1_tags_1[0]), /* 1 */
	asn_DEF_RoutingInfoForSM_Res_v1_tags_1,	/* Same as above */
	sizeof(asn_DEF_RoutingInfoForSM_Res_v1_tags_1)
		/sizeof(asn_DEF_RoutingInfoForSM_Res_v1_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_RoutingInfoForSM_Res_v1_1,
	3,	/* Elements count */
	&asn_SPC_RoutingInfoForSM_Res_v1_specs_1	/* Additional specs */
};

