#include <asn_internal.h>

#include "USSD-Res.h"

static asn_TYPE_member_t asn_MBR_USSD_Res_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct USSD_Res, ussd_DataCodingScheme),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_USSD_DataCodingScheme,
		0,	/* Defer constraints checking to the member type */
		"ussd-DataCodingScheme"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct USSD_Res, ussd_String),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_USSD_String,
		0,	/* Defer constraints checking to the member type */
		"ussd-String"
		},
};
static ber_tlv_tag_t asn_DEF_USSD_Res_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_USSD_Res_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 1 }, /* ussd-DataCodingScheme at 100 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, -1, 0 } /* ussd-String at 101 */
};
static asn_SEQUENCE_specifics_t asn_SPC_USSD_Res_1_specs = {
	sizeof(struct USSD_Res),
	offsetof(struct USSD_Res, _asn_ctx),
	asn_MAP_USSD_Res_1_tag2el,
	2,	/* Count of tags in the map */
	1,	/* Start extensions */
	3	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_USSD_Res = {
	"USSD-Res",
	"USSD-Res",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_USSD_Res_1_tags,
	sizeof(asn_DEF_USSD_Res_1_tags)
		/sizeof(asn_DEF_USSD_Res_1_tags[0]), /* 1 */
	asn_DEF_USSD_Res_1_tags,	/* Same as above */
	sizeof(asn_DEF_USSD_Res_1_tags)
		/sizeof(asn_DEF_USSD_Res_1_tags[0]), /* 1 */
	asn_MBR_USSD_Res_1,
	2,	/* Elements count */
	&asn_SPC_USSD_Res_1_specs	/* Additional specs */
};

