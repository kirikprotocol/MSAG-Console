#ident "$Id$"

#include <asn_internal.h>

#include "SeqType1.h"

static asn_TYPE_member_t asn_MBR_SeqType1_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SeqType1, a),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		"a"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SeqType1, b),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		"b"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SeqType1, c),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"c"
		},
};
static ber_tlv_tag_t asn_DEF_SeqType1_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SeqType1_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* a at 11 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 2, 0, 0 }, /* c at 13 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 1, 0, 0 } /* b at 12 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SeqType1_1_specs = {
	sizeof(struct SeqType1),
	offsetof(struct SeqType1, _asn_ctx),
	asn_MAP_SeqType1_1_tag2el,
	3,	/* Count of tags in the map */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SeqType1 = {
	"SeqType1",
	"SeqType1",
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
	asn_DEF_SeqType1_1_tags,
	sizeof(asn_DEF_SeqType1_1_tags)
		/sizeof(asn_DEF_SeqType1_1_tags[0]), /* 1 */
	asn_DEF_SeqType1_1_tags,	/* Same as above */
	sizeof(asn_DEF_SeqType1_1_tags)
		/sizeof(asn_DEF_SeqType1_1_tags[0]), /* 1 */
	asn_MBR_SeqType1_1,
	3,	/* Elements count */
	&asn_SPC_SeqType1_1_specs	/* Additional specs */
};

