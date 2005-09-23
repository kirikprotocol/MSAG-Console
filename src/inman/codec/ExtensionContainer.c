#include <asn_internal.h>

#include "ExtensionContainer.h"

static asn_TYPE_member_t asn_MBR_ExtensionContainer_1[] = {
	{ ATF_POINTER, 2, offsetof(struct ExtensionContainer, privateExtensionList),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrivateExtensionList,
		0,	/* Defer constraints checking to the member type */
		"privateExtensionList"
		},
	{ ATF_POINTER, 1, offsetof(struct ExtensionContainer, pcs_Extensions),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PCS_Extensions,
		0,	/* Defer constraints checking to the member type */
		"pcs-Extensions"
		},
};
static ber_tlv_tag_t asn_DEF_ExtensionContainer_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ExtensionContainer_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* privateExtensionList at 435 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* pcs-Extensions at 436 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ExtensionContainer_1_specs = {
	sizeof(struct ExtensionContainer),
	offsetof(struct ExtensionContainer, _asn_ctx),
	asn_MAP_ExtensionContainer_1_tag2el,
	2,	/* Count of tags in the map */
	1,	/* Start extensions */
	3	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ExtensionContainer = {
	"ExtensionContainer",
	"ExtensionContainer",
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
	asn_DEF_ExtensionContainer_1_tags,
	sizeof(asn_DEF_ExtensionContainer_1_tags)
		/sizeof(asn_DEF_ExtensionContainer_1_tags[0]), /* 1 */
	asn_DEF_ExtensionContainer_1_tags,	/* Same as above */
	sizeof(asn_DEF_ExtensionContainer_1_tags)
		/sizeof(asn_DEF_ExtensionContainer_1_tags[0]), /* 1 */
	asn_MBR_ExtensionContainer_1,
	2,	/* Elements count */
	&asn_SPC_ExtensionContainer_1_specs	/* Additional specs */
};

