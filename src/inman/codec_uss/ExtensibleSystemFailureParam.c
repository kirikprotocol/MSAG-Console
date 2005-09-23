#include <asn_internal.h>

#include "ExtensibleSystemFailureParam.h"

static asn_TYPE_member_t asn_MBR_ExtensibleSystemFailureParam_1[] = {
	{ ATF_POINTER, 3, offsetof(struct ExtensibleSystemFailureParam, networkResource),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_NetworkResource,
		0,	/* Defer constraints checking to the member type */
		"networkResource"
		},
	{ ATF_POINTER, 2, offsetof(struct ExtensibleSystemFailureParam, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 1, offsetof(struct ExtensibleSystemFailureParam, additionalNetworkResource),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AdditionalNetworkResource,
		0,	/* Defer constraints checking to the member type */
		"additionalNetworkResource"
		},
};
static ber_tlv_tag_t asn_DEF_ExtensibleSystemFailureParam_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ExtensibleSystemFailureParam_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* networkResource at 278 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 }, /* extensionContainer at 279 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 2, 0, 0 } /* additionalNetworkResource at 281 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ExtensibleSystemFailureParam_1_specs = {
	sizeof(struct ExtensibleSystemFailureParam),
	offsetof(struct ExtensibleSystemFailureParam, _asn_ctx),
	asn_MAP_ExtensibleSystemFailureParam_1_tag2el,
	3,	/* Count of tags in the map */
	1,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ExtensibleSystemFailureParam = {
	"ExtensibleSystemFailureParam",
	"ExtensibleSystemFailureParam",
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
	asn_DEF_ExtensibleSystemFailureParam_1_tags,
	sizeof(asn_DEF_ExtensibleSystemFailureParam_1_tags)
		/sizeof(asn_DEF_ExtensibleSystemFailureParam_1_tags[0]), /* 1 */
	asn_DEF_ExtensibleSystemFailureParam_1_tags,	/* Same as above */
	sizeof(asn_DEF_ExtensibleSystemFailureParam_1_tags)
		/sizeof(asn_DEF_ExtensibleSystemFailureParam_1_tags[0]), /* 1 */
	asn_MBR_ExtensibleSystemFailureParam_1,
	3,	/* Elements count */
	&asn_SPC_ExtensibleSystemFailureParam_1_specs	/* Additional specs */
};

