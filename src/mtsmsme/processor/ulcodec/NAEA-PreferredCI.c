#include <asn_internal.h>

#include "NAEA-PreferredCI.h"

static asn_TYPE_member_t asn_MBR_NAEA_PreferredCI_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct NAEA_PreferredCI, naea_PreferredCIC),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NAEA_CIC,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"naea-PreferredCIC"
		},
	{ ATF_POINTER, 1, offsetof(struct NAEA_PreferredCI, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_NAEA_PreferredCI_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_NAEA_PreferredCI_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* naea-PreferredCIC at 284 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* extensionContainer at 285 */
};
static asn_SEQUENCE_specifics_t asn_SPC_NAEA_PreferredCI_specs_1 = {
	sizeof(struct NAEA_PreferredCI),
	offsetof(struct NAEA_PreferredCI, _asn_ctx),
	asn_MAP_NAEA_PreferredCI_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* Start extensions */
	3	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_NAEA_PreferredCI = {
	"NAEA-PreferredCI",
	"NAEA-PreferredCI",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_NAEA_PreferredCI_tags_1,
	sizeof(asn_DEF_NAEA_PreferredCI_tags_1)
		/sizeof(asn_DEF_NAEA_PreferredCI_tags_1[0]), /* 1 */
	asn_DEF_NAEA_PreferredCI_tags_1,	/* Same as above */
	sizeof(asn_DEF_NAEA_PreferredCI_tags_1)
		/sizeof(asn_DEF_NAEA_PreferredCI_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_NAEA_PreferredCI_1,
	2,	/* Elements count */
	&asn_SPC_NAEA_PreferredCI_specs_1	/* Additional specs */
};

