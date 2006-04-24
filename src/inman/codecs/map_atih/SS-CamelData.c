#ident "$Id$"

#include <asn_internal.h>

#include "SS-CamelData.h"

static asn_TYPE_member_t asn_MBR_SS_CamelData_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SS_CamelData, ss_EventList),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_SS_EventList,
		0,	/* Defer constraints checking to the member type */
		"ss-EventList"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SS_CamelData, gsmSCF_Address),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"gsmSCF-Address"
		},
	{ ATF_POINTER, 1, offsetof(struct SS_CamelData, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_SS_CamelData_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SS_CamelData_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, 0, 0 }, /* gsmSCF-Address at 454 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 0 }, /* ss-EventList at 453 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 2, 0, 0 } /* extensionContainer at 455 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SS_CamelData_1_specs = {
	sizeof(struct SS_CamelData),
	offsetof(struct SS_CamelData, _asn_ctx),
	asn_MAP_SS_CamelData_1_tag2el,
	3,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SS_CamelData = {
	"SS-CamelData",
	"SS-CamelData",
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
	asn_DEF_SS_CamelData_1_tags,
	sizeof(asn_DEF_SS_CamelData_1_tags)
		/sizeof(asn_DEF_SS_CamelData_1_tags[0]), /* 1 */
	asn_DEF_SS_CamelData_1_tags,	/* Same as above */
	sizeof(asn_DEF_SS_CamelData_1_tags)
		/sizeof(asn_DEF_SS_CamelData_1_tags[0]), /* 1 */
	asn_MBR_SS_CamelData_1,
	3,	/* Elements count */
	&asn_SPC_SS_CamelData_1_specs	/* Additional specs */
};

