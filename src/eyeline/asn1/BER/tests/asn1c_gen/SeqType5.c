#ident "$Id$"

#include <asn_internal.h>

#include "SeqType5.h"

static asn_TYPE_member_t asn_MBR_SeqType5_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SeqType5, a),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_ChoiceType1,
		0,	/* Defer constraints checking to the member type */
		"a"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SeqType5, b),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		0,
		&asn_DEF_ChoiceType2,
		0,	/* Defer constraints checking to the member type */
		"b"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SeqType5, c),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_ChoiceType1,
		0,	/* Defer constraints checking to the member type */
		"c"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SeqType5, d),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ChoiceType2,
		0,	/* Defer constraints checking to the member type */
		"d"
		},
};
static ber_tlv_tag_t asn_DEF_SeqType5_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SeqType5_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (1 << 2)), 0, 0, 0 }, /* case2 at 41 */
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* case1 at 40 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 2, 0, 0 }, /* c at 52 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 3, 0, 0 }, /* d at 54 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 1, 0, 0 } /* b at 51 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SeqType5_1_specs = {
	sizeof(struct SeqType5),
	offsetof(struct SeqType5, _asn_ctx),
	asn_MAP_SeqType5_1_tag2el,
	5,	/* Count of tags in the map */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SeqType5 = {
	"SeqType5",
	"SeqType5",
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
	asn_DEF_SeqType5_1_tags,
	sizeof(asn_DEF_SeqType5_1_tags)
		/sizeof(asn_DEF_SeqType5_1_tags[0]), /* 1 */
	asn_DEF_SeqType5_1_tags,	/* Same as above */
	sizeof(asn_DEF_SeqType5_1_tags)
		/sizeof(asn_DEF_SeqType5_1_tags[0]), /* 1 */
	asn_MBR_SeqType5_1,
	4,	/* Elements count */
	&asn_SPC_SeqType5_1_specs	/* Additional specs */
};

