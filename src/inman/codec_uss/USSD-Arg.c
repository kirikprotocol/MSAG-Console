#include <asn_internal.h>

#include "USSD-Arg.h"

static asn_TYPE_member_t asn_MBR_USSD_Arg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct USSD_Arg, ussd_DataCodingScheme),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_USSD_DataCodingScheme,
		0,	/* Defer constraints checking to the member type */
		"ussd-DataCodingScheme"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct USSD_Arg, ussd_String),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_USSD_String,
		0,	/* Defer constraints checking to the member type */
		"ussd-String"
		},
	{ ATF_POINTER, 2, offsetof(struct USSD_Arg, alertingPattern),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_AlertingPattern,
		0,	/* Defer constraints checking to the member type */
		"alertingPattern"
		},
	{ ATF_POINTER, 1, offsetof(struct USSD_Arg, msisdn),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"msisdn"
		},
};
static ber_tlv_tag_t asn_DEF_USSD_Arg_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_USSD_Arg_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 2 }, /* ussd-DataCodingScheme at 93 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, -1, 1 }, /* ussd-String at 94 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 2, -2, 0 }, /* alertingPattern at 96 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 3, 0, 0 } /* msisdn at 97 */
};
static asn_SEQUENCE_specifics_t asn_SPC_USSD_Arg_1_specs = {
	sizeof(struct USSD_Arg),
	offsetof(struct USSD_Arg, _asn_ctx),
	asn_MAP_USSD_Arg_1_tag2el,
	4,	/* Count of tags in the map */
	1,	/* Start extensions */
	5	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_USSD_Arg = {
	"USSD-Arg",
	"USSD-Arg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_USSD_Arg_1_tags,
	sizeof(asn_DEF_USSD_Arg_1_tags)
		/sizeof(asn_DEF_USSD_Arg_1_tags[0]), /* 1 */
	asn_DEF_USSD_Arg_1_tags,	/* Same as above */
	sizeof(asn_DEF_USSD_Arg_1_tags)
		/sizeof(asn_DEF_USSD_Arg_1_tags[0]), /* 1 */
	asn_MBR_USSD_Arg_1,
	4,	/* Elements count */
	&asn_SPC_USSD_Arg_1_specs	/* Additional specs */
};

