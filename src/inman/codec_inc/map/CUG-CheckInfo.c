#ident "$Id$"

#include <asn_internal.h>

#include "CUG-CheckInfo.h"

static asn_TYPE_member_t asn_MBR_CUG_CheckInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CUG_CheckInfo, cug_Interlock),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_CUG_Interlock,
		0,	/* Defer constraints checking to the member type */
		"cug-Interlock"
		},
	{ ATF_POINTER, 2, offsetof(struct CUG_CheckInfo, cug_OutgoingAccess),
		(ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
		0,
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"cug-OutgoingAccess"
		},
	{ ATF_POINTER, 1, offsetof(struct CUG_CheckInfo, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_CUG_CheckInfo_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CUG_CheckInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* cug-Interlock at 66 */
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 1, 0, 0 }, /* cug-OutgoingAccess at 67 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, 0, 0 } /* extensionContainer at 68 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CUG_CheckInfo_1_specs = {
	sizeof(struct CUG_CheckInfo),
	offsetof(struct CUG_CheckInfo, _asn_ctx),
	asn_MAP_CUG_CheckInfo_1_tag2el,
	3,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CUG_CheckInfo = {
	"CUG-CheckInfo",
	"CUG-CheckInfo",
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
	asn_DEF_CUG_CheckInfo_1_tags,
	sizeof(asn_DEF_CUG_CheckInfo_1_tags)
		/sizeof(asn_DEF_CUG_CheckInfo_1_tags[0]), /* 1 */
	asn_DEF_CUG_CheckInfo_1_tags,	/* Same as above */
	sizeof(asn_DEF_CUG_CheckInfo_1_tags)
		/sizeof(asn_DEF_CUG_CheckInfo_1_tags[0]), /* 1 */
	asn_MBR_CUG_CheckInfo_1,
	3,	/* Elements count */
	&asn_SPC_CUG_CheckInfo_1_specs	/* Additional specs */
};

