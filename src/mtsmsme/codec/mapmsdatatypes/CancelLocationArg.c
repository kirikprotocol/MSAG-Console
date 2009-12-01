#include <asn_internal.h>

#include "CancelLocationArg.h"

static asn_TYPE_member_t asn_MBR_CancelLocationArg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CancelLocationArg, identity),
	  (ber_tlv_tag_t)-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_Identity,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"identity"
		},
	{ ATF_POINTER, 2, offsetof(struct CancelLocationArg, cancellationType),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_CancellationType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cancellationType"
		},
	{ ATF_POINTER, 1, offsetof(struct CancelLocationArg, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_CancelLocationArg_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CancelLocationArg_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* imsi at 253 */
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 1, 0, 0 }, /* cancellationType at 144 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 1 }, /* imsi-WithLMSI at 255 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, -1, 0 } /* extensionContainer at 145 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CancelLocationArg_specs_1 = {
	sizeof(struct CancelLocationArg),
	offsetof(struct CancelLocationArg, _asn_ctx),
	asn_MAP_CancelLocationArg_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CancelLocationArg = {
	"CancelLocationArg",
	"CancelLocationArg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CancelLocationArg_tags_1,
	sizeof(asn_DEF_CancelLocationArg_tags_1)
		/sizeof(asn_DEF_CancelLocationArg_tags_1[0]) - 1, /* 1 */
	asn_DEF_CancelLocationArg_tags_1,	/* Same as above */
	sizeof(asn_DEF_CancelLocationArg_tags_1)
		/sizeof(asn_DEF_CancelLocationArg_tags_1[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_CancelLocationArg_1,
	3,	/* Elements count */
	&asn_SPC_CancelLocationArg_specs_1	/* Additional specs */
};

