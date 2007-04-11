#include <asn_internal.h>

#include "ExtensibleCallBarredParam.h"

static asn_TYPE_member_t asn_MBR_ExtensibleCallBarredParam_1[] = {
	{ ATF_POINTER, 3, offsetof(struct ExtensibleCallBarredParam, callBarringCause),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_CallBarringCause,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"callBarringCause"
		},
	{ ATF_POINTER, 2, offsetof(struct ExtensibleCallBarredParam, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 1, offsetof(struct ExtensibleCallBarredParam, unauthorisedMessageOriginator),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"unauthorisedMessageOriginator"
		},
};
static ber_tlv_tag_t asn_DEF_ExtensibleCallBarredParam_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ExtensibleCallBarredParam_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* callBarringCause at 79 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 }, /* extensionContainer at 80 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 2, 0, 0 } /* unauthorisedMessageOriginator at 82 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ExtensibleCallBarredParam_specs_1 = {
	sizeof(struct ExtensibleCallBarredParam),
	offsetof(struct ExtensibleCallBarredParam, _asn_ctx),
	asn_MAP_ExtensibleCallBarredParam_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ExtensibleCallBarredParam = {
	"ExtensibleCallBarredParam",
	"ExtensibleCallBarredParam",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ExtensibleCallBarredParam_tags_1,
	sizeof(asn_DEF_ExtensibleCallBarredParam_tags_1)
		/sizeof(asn_DEF_ExtensibleCallBarredParam_tags_1[0]), /* 1 */
	asn_DEF_ExtensibleCallBarredParam_tags_1,	/* Same as above */
	sizeof(asn_DEF_ExtensibleCallBarredParam_tags_1)
		/sizeof(asn_DEF_ExtensibleCallBarredParam_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_ExtensibleCallBarredParam_1,
	3,	/* Elements count */
	&asn_SPC_ExtensibleCallBarredParam_specs_1	/* Additional specs */
};

