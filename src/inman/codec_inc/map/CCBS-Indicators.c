#ident "$Id$"

#include <asn_internal.h>

#include "CCBS-Indicators.h"

static asn_TYPE_member_t asn_MBR_CCBS_Indicators_1[] = {
	{ ATF_POINTER, 3, offsetof(struct CCBS_Indicators, ccbs_Possible),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"ccbs-Possible"
		},
	{ ATF_POINTER, 2, offsetof(struct CCBS_Indicators, keepCCBS_CallIndicator),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"keepCCBS-CallIndicator"
		},
	{ ATF_POINTER, 1, offsetof(struct CCBS_Indicators, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_CCBS_Indicators_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CCBS_Indicators_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* ccbs-Possible at 181 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* keepCCBS-CallIndicator at 182 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* extensionContainer at 183 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CCBS_Indicators_1_specs = {
	sizeof(struct CCBS_Indicators),
	offsetof(struct CCBS_Indicators, _asn_ctx),
	asn_MAP_CCBS_Indicators_1_tag2el,
	3,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CCBS_Indicators = {
	"CCBS-Indicators",
	"CCBS-Indicators",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
#ifndef ASN1_XER_NOT_USED
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CCBS_Indicators_1_tags,
	sizeof(asn_DEF_CCBS_Indicators_1_tags)
		/sizeof(asn_DEF_CCBS_Indicators_1_tags[0]), /* 1 */
	asn_DEF_CCBS_Indicators_1_tags,	/* Same as above */
	sizeof(asn_DEF_CCBS_Indicators_1_tags)
		/sizeof(asn_DEF_CCBS_Indicators_1_tags[0]), /* 1 */
	asn_MBR_CCBS_Indicators_1,
	3,	/* Elements count */
	&asn_SPC_CCBS_Indicators_1_specs	/* Additional specs */
};

