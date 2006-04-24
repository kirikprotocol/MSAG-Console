#ident "$Id$"

#include <asn_internal.h>

#include "SS-ForBS-Code.h"

static asn_TYPE_member_t asn_MBR_SS_ForBS_Code_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SS_ForBS_Code, ss_Code),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_SS_Code,
		0,	/* Defer constraints checking to the member type */
		"ss-Code"
		},
	{ ATF_POINTER, 2, offsetof(struct SS_ForBS_Code, basicService),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_BasicServiceCode,
		0,	/* Defer constraints checking to the member type */
		"basicService"
		},
	{ ATF_POINTER, 1, offsetof(struct SS_ForBS_Code, longFTN_Supported),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"longFTN-Supported"
		},
};
static ber_tlv_tag_t asn_DEF_SS_ForBS_Code_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SS_ForBS_Code_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* ss-Code at 48 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* bearerService at 263 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 1, 0, 0 }, /* teleservice at 265 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 2, 0, 0 } /* longFTN-Supported at 51 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SS_ForBS_Code_1_specs = {
	sizeof(struct SS_ForBS_Code),
	offsetof(struct SS_ForBS_Code, _asn_ctx),
	asn_MAP_SS_ForBS_Code_1_tag2el,
	4,	/* Count of tags in the map */
	1,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SS_ForBS_Code = {
	"SS-ForBS-Code",
	"SS-ForBS-Code",
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
	asn_DEF_SS_ForBS_Code_1_tags,
	sizeof(asn_DEF_SS_ForBS_Code_1_tags)
		/sizeof(asn_DEF_SS_ForBS_Code_1_tags[0]), /* 1 */
	asn_DEF_SS_ForBS_Code_1_tags,	/* Same as above */
	sizeof(asn_DEF_SS_ForBS_Code_1_tags)
		/sizeof(asn_DEF_SS_ForBS_Code_1_tags[0]), /* 1 */
	asn_MBR_SS_ForBS_Code_1,
	3,	/* Elements count */
	&asn_SPC_SS_ForBS_Code_1_specs	/* Additional specs */
};

