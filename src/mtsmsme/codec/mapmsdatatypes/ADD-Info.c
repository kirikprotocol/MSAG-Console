#include <asn_internal.h>

#include "ADD-Info.h"

static asn_TYPE_member_t asn_MBR_ADD_Info_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ADD_Info, imeisv),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMEI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imeisv"
		},
	{ ATF_POINTER, 1, offsetof(struct ADD_Info, skipSubscriberDataUpdate),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"skipSubscriberDataUpdate"
		},
};
static ber_tlv_tag_t asn_DEF_ADD_Info_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ADD_Info_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* imeisv at 137 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* skipSubscriberDataUpdate at 138 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ADD_Info_specs_1 = {
	sizeof(struct ADD_Info),
	offsetof(struct ADD_Info, _asn_ctx),
	asn_MAP_ADD_Info_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* Start extensions */
	3	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ADD_Info = {
	"ADD-Info",
	"ADD-Info",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ADD_Info_tags_1,
	sizeof(asn_DEF_ADD_Info_tags_1)
		/sizeof(asn_DEF_ADD_Info_tags_1[0]), /* 1 */
	asn_DEF_ADD_Info_tags_1,	/* Same as above */
	sizeof(asn_DEF_ADD_Info_tags_1)
		/sizeof(asn_DEF_ADD_Info_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_ADD_Info_1,
	2,	/* Elements count */
	&asn_SPC_ADD_Info_specs_1	/* Additional specs */
};

