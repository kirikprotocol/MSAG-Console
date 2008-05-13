#include <asn_internal.h>

#include "MO-ForwardSM-Arg.h"

static asn_TYPE_member_t asn_MBR_MO_ForwardSM_Arg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct MO_ForwardSM_Arg, sm_RP_DA),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_SM_RP_DA,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-DA"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MO_ForwardSM_Arg, sm_RP_OA),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_SM_RP_OA,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-OA"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MO_ForwardSM_Arg, sm_RP_UI),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_SignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-UI"
		},
	{ ATF_POINTER, 2, offsetof(struct MO_ForwardSM_Arg, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 1, offsetof(struct MO_ForwardSM_Arg, imsi),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imsi"
		},
};
static ber_tlv_tag_t asn_DEF_MO_ForwardSM_Arg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_MO_ForwardSM_Arg_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 2, 0, 1 }, /* sm-RP-UI at 115 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 4, -1, 0 }, /* imsi at 118 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 3, 0, 0 }, /* extensionContainer at 116 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* imsi at 139 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* lmsi at 140 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* msisdn at 145 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 0, 0, 1 }, /* serviceCentreAddressDA at 141 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 1, -1, 0 }, /* serviceCentreAddressOA at 146 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 0, 0, 1 }, /* noSM-RP-DA at 142 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 1, -1, 0 } /* noSM-RP-OA at 147 */
};
static asn_SEQUENCE_specifics_t asn_SPC_MO_ForwardSM_Arg_specs_1 = {
	sizeof(struct MO_ForwardSM_Arg),
	offsetof(struct MO_ForwardSM_Arg, _asn_ctx),
	asn_MAP_MO_ForwardSM_Arg_tag2el_1,
	10,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	3,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_MO_ForwardSM_Arg = {
	"MO-ForwardSM-Arg",
	"MO-ForwardSM-Arg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_MO_ForwardSM_Arg_tags_1,
	sizeof(asn_DEF_MO_ForwardSM_Arg_tags_1)
		/sizeof(asn_DEF_MO_ForwardSM_Arg_tags_1[0]), /* 1 */
	asn_DEF_MO_ForwardSM_Arg_tags_1,	/* Same as above */
	sizeof(asn_DEF_MO_ForwardSM_Arg_tags_1)
		/sizeof(asn_DEF_MO_ForwardSM_Arg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_MO_ForwardSM_Arg_1,
	5,	/* Elements count */
	&asn_SPC_MO_ForwardSM_Arg_specs_1	/* Additional specs */
};

