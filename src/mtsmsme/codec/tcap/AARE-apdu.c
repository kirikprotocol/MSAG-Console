#include <asn_internal.h>

#include "AARE-apdu.h"

static asn_TYPE_member_t asn_MBR_aare_user_information_6[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (8 << 2)),
		0,
		&asn_DEF_MEXT,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static ber_tlv_tag_t asn_DEF_aare_user_information_tags_6[] = {
	(ASN_TAG_CLASS_CONTEXT | (30 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_aare_user_information_specs_6 = {
	sizeof(struct aare_user_information),
	offsetof(struct aare_user_information, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_aare_user_information_6 = {
	"aare-user-information",
	"aare-user-information",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_aare_user_information_tags_6,
	sizeof(asn_DEF_aare_user_information_tags_6)
		/sizeof(asn_DEF_aare_user_information_tags_6[0]) - 1, /* 1 */
	asn_DEF_aare_user_information_tags_6,	/* Same as above */
	sizeof(asn_DEF_aare_user_information_tags_6)
		/sizeof(asn_DEF_aare_user_information_tags_6[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_aare_user_information_6,
	1,	/* Single element */
	&asn_SPC_aare_user_information_specs_6	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_AARE_apdu_1[] = {
	{ ATF_POINTER, 1, offsetof(struct AARE_apdu, protocol_version),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BIT_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"protocol-version"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AARE_apdu, application_context_name),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_OBJECT_IDENTIFIER,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"application-context-name"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AARE_apdu, result),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_Associate_result,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"result"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AARE_apdu, result_source_diagnostic),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_Associate_source_diagnostic,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"result-source-diagnostic"
		},
	{ ATF_POINTER, 1, offsetof(struct AARE_apdu, aare_user_information),
		(ASN_TAG_CLASS_CONTEXT | (30 << 2)),
		0,
		&asn_DEF_aare_user_information_6,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"aare-user-information"
		},
};
static ber_tlv_tag_t asn_DEF_AARE_apdu_tags_1[] = {
	(ASN_TAG_CLASS_APPLICATION | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_AARE_apdu_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* protocol-version at 28 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* application-context-name at 29 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* result at 30 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* result-source-diagnostic at 31 */
    { (ASN_TAG_CLASS_CONTEXT | (30 << 2)), 4, 0, 0 } /* aare-user-information at 32 */
};
static asn_SEQUENCE_specifics_t asn_SPC_AARE_apdu_specs_1 = {
	sizeof(struct AARE_apdu),
	offsetof(struct AARE_apdu, _asn_ctx),
	asn_MAP_AARE_apdu_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_AARE_apdu = {
	"AARE-apdu",
	"AARE-apdu",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_AARE_apdu_tags_1,
	sizeof(asn_DEF_AARE_apdu_tags_1)
		/sizeof(asn_DEF_AARE_apdu_tags_1[0]) - 1, /* 1 */
	asn_DEF_AARE_apdu_tags_1,	/* Same as above */
	sizeof(asn_DEF_AARE_apdu_tags_1)
		/sizeof(asn_DEF_AARE_apdu_tags_1[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_AARE_apdu_1,
	5,	/* Elements count */
	&asn_SPC_AARE_apdu_specs_1	/* Additional specs */
};

