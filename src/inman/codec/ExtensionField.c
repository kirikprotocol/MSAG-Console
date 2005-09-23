#include <asn_internal.h>

#include "ExtensionField.h"

static asn_TYPE_member_t asn_MBR_ExtensionField_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ExtensionField, type),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_Code,
		0,	/* Defer constraints checking to the member type */
		"type"
		},
	{ ATF_POINTER, 1, offsetof(struct ExtensionField, criticality),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_CriticalityType,
		0,	/* Defer constraints checking to the member type */
		"criticality"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ExtensionField, value),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,	/* Defer constraints checking to the member type */
		"value"
		},
};
static ber_tlv_tag_t asn_DEF_ExtensionField_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ExtensionField_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* local at 127 */
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 0, 0, 0 }, /* global at 128 */
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 1, 0, 0 }, /* criticality at 643 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 2, 0, 0 } /* value at 645 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ExtensionField_1_specs = {
	sizeof(struct ExtensionField),
	offsetof(struct ExtensionField, _asn_ctx),
	asn_MAP_ExtensionField_1_tag2el,
	4,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ExtensionField = {
	"ExtensionField",
	"ExtensionField",
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
	asn_DEF_ExtensionField_1_tags,
	sizeof(asn_DEF_ExtensionField_1_tags)
		/sizeof(asn_DEF_ExtensionField_1_tags[0]), /* 1 */
	asn_DEF_ExtensionField_1_tags,	/* Same as above */
	sizeof(asn_DEF_ExtensionField_1_tags)
		/sizeof(asn_DEF_ExtensionField_1_tags[0]), /* 1 */
	asn_MBR_ExtensionField_1,
	3,	/* Elements count */
	&asn_SPC_ExtensionField_1_specs	/* Additional specs */
};

