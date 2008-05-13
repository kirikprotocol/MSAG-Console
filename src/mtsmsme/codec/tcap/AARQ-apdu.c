#include <asn_internal.h>

#include "AARQ-apdu.h"

static asn_TYPE_member_t asn_MBR_aarq_user_information_4[] = {
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
static ber_tlv_tag_t asn_DEF_aarq_user_information_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (30 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_aarq_user_information_specs_4 = {
	sizeof(struct aarq_user_information),
	offsetof(struct aarq_user_information, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_aarq_user_information_4 = {
	"aarq-user-information",
	"aarq-user-information",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_aarq_user_information_tags_4,
	sizeof(asn_DEF_aarq_user_information_tags_4)
		/sizeof(asn_DEF_aarq_user_information_tags_4[0]) - 1, /* 1 */
	asn_DEF_aarq_user_information_tags_4,	/* Same as above */
	sizeof(asn_DEF_aarq_user_information_tags_4)
		/sizeof(asn_DEF_aarq_user_information_tags_4[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_aarq_user_information_4,
	1,	/* Single element */
	&asn_SPC_aarq_user_information_specs_4	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_AARQ_apdu_1[] = {
	{ ATF_POINTER, 1, offsetof(struct AARQ_apdu, protocol_version),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BIT_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"protocol-version"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AARQ_apdu, application_context_name),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_OBJECT_IDENTIFIER,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"application-context-name"
		},
	{ ATF_POINTER, 1, offsetof(struct AARQ_apdu, aarq_user_information),
		(ASN_TAG_CLASS_CONTEXT | (30 << 2)),
		0,
		&asn_DEF_aarq_user_information_4,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"aarq-user-information"
		},
};
static ber_tlv_tag_t asn_DEF_AARQ_apdu_tags_1[] = {
	(ASN_TAG_CLASS_APPLICATION | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_AARQ_apdu_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* protocol-version at 21 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* application-context-name at 22 */
    { (ASN_TAG_CLASS_CONTEXT | (30 << 2)), 2, 0, 0 } /* aarq-user-information at 23 */
};
static asn_SEQUENCE_specifics_t asn_SPC_AARQ_apdu_specs_1 = {
	sizeof(struct AARQ_apdu),
	offsetof(struct AARQ_apdu, _asn_ctx),
	asn_MAP_AARQ_apdu_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_AARQ_apdu = {
	"AARQ-apdu",
	"AARQ-apdu",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_AARQ_apdu_tags_1,
	sizeof(asn_DEF_AARQ_apdu_tags_1)
		/sizeof(asn_DEF_AARQ_apdu_tags_1[0]) - 1, /* 1 */
	asn_DEF_AARQ_apdu_tags_1,	/* Same as above */
	sizeof(asn_DEF_AARQ_apdu_tags_1)
		/sizeof(asn_DEF_AARQ_apdu_tags_1[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_AARQ_apdu_1,
	3,	/* Elements count */
	&asn_SPC_AARQ_apdu_specs_1	/* Additional specs */
};

