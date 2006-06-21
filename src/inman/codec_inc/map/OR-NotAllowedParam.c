#ident "$Id$"

#include <asn_internal.h>

#include "OR-NotAllowedParam.h"

static asn_TYPE_member_t asn_MBR_OR_NotAllowedParam_1[] = {
	{ ATF_POINTER, 1, offsetof(struct OR_NotAllowedParam, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_OR_NotAllowedParam_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_OR_NotAllowedParam_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 0 } /* extensionContainer at 120 */
};
static asn_SEQUENCE_specifics_t asn_SPC_OR_NotAllowedParam_1_specs = {
	sizeof(struct OR_NotAllowedParam),
	offsetof(struct OR_NotAllowedParam, _asn_ctx),
	asn_MAP_OR_NotAllowedParam_1_tag2el,
	1,	/* Count of tags in the map */
	0,	/* Start extensions */
	2	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_OR_NotAllowedParam = {
	"OR-NotAllowedParam",
	"OR-NotAllowedParam",
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
	asn_DEF_OR_NotAllowedParam_1_tags,
	sizeof(asn_DEF_OR_NotAllowedParam_1_tags)
		/sizeof(asn_DEF_OR_NotAllowedParam_1_tags[0]), /* 1 */
	asn_DEF_OR_NotAllowedParam_1_tags,	/* Same as above */
	sizeof(asn_DEF_OR_NotAllowedParam_1_tags)
		/sizeof(asn_DEF_OR_NotAllowedParam_1_tags[0]), /* 1 */
	asn_MBR_OR_NotAllowedParam_1,
	1,	/* Elements count */
	&asn_SPC_OR_NotAllowedParam_1_specs	/* Additional specs */
};

