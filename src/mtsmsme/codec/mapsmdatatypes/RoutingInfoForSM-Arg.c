#include <asn_internal.h>

#include "RoutingInfoForSM-Arg.h"

static asn_TYPE_member_t asn_MBR_RoutingInfoForSM_Arg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfoForSM_Arg, msisdn),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msisdn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfoForSM_Arg, sm_RP_PRI),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-PRI"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfoForSM_Arg, serviceCentreAddress),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceCentreAddress"
		},
	{ ATF_POINTER, 4, offsetof(struct RoutingInfoForSM_Arg, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 3, offsetof(struct RoutingInfoForSM_Arg, gprsSupportIndicator),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gprsSupportIndicator"
		},
	{ ATF_POINTER, 2, offsetof(struct RoutingInfoForSM_Arg, sm_RP_MTI),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SM_RP_MTI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-MTI"
		},
	{ ATF_POINTER, 1, offsetof(struct RoutingInfoForSM_Arg, sm_RP_SMEA),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SM_RP_SMEA,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-SMEA"
		},
};
static ber_tlv_tag_t asn_DEF_RoutingInfoForSM_Arg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_RoutingInfoForSM_Arg_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* msisdn at 61 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* sm-RP-PRI at 62 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* serviceCentreAddress at 63 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 3, 0, 0 }, /* extensionContainer at 64 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 4, 0, 0 }, /* gprsSupportIndicator at 66 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 5, 0, 0 }, /* sm-RP-MTI at 69 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 6, 0, 0 } /* sm-RP-SMEA at 70 */
};
static asn_SEQUENCE_specifics_t asn_SPC_RoutingInfoForSM_Arg_specs_1 = {
	sizeof(struct RoutingInfoForSM_Arg),
	offsetof(struct RoutingInfoForSM_Arg, _asn_ctx),
	asn_MAP_RoutingInfoForSM_Arg_tag2el_1,
	7,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	3,	/* Start extensions */
	8	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_RoutingInfoForSM_Arg = {
	"RoutingInfoForSM-Arg",
	"RoutingInfoForSM-Arg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_RoutingInfoForSM_Arg_tags_1,
	sizeof(asn_DEF_RoutingInfoForSM_Arg_tags_1)
		/sizeof(asn_DEF_RoutingInfoForSM_Arg_tags_1[0]), /* 1 */
	asn_DEF_RoutingInfoForSM_Arg_tags_1,	/* Same as above */
	sizeof(asn_DEF_RoutingInfoForSM_Arg_tags_1)
		/sizeof(asn_DEF_RoutingInfoForSM_Arg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_RoutingInfoForSM_Arg_1,
	7,	/* Elements count */
	&asn_SPC_RoutingInfoForSM_Arg_specs_1	/* Additional specs */
};

