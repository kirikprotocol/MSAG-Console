#include <asn_internal.h>

#include "UpdateLocationRes.h"

static asn_TYPE_member_t asn_MBR_UpdateLocationRes_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct UpdateLocationRes, hlr_Number),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"hlr-Number"
		},
	{ ATF_POINTER, 2, offsetof(struct UpdateLocationRes, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 1, offsetof(struct UpdateLocationRes, add_Capability),
		(ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
		0,
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"add-Capability"
		},
};
static ber_tlv_tag_t asn_DEF_UpdateLocationRes_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_UpdateLocationRes_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* hlr-Number at 98 */
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 2, 0, 0 }, /* add-Capability at 101 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* extensionContainer at 99 */
};
static asn_SEQUENCE_specifics_t asn_SPC_UpdateLocationRes_specs_1 = {
	sizeof(struct UpdateLocationRes),
	offsetof(struct UpdateLocationRes, _asn_ctx),
	asn_MAP_UpdateLocationRes_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_UpdateLocationRes = {
	"UpdateLocationRes",
	"UpdateLocationRes",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_UpdateLocationRes_tags_1,
	sizeof(asn_DEF_UpdateLocationRes_tags_1)
		/sizeof(asn_DEF_UpdateLocationRes_tags_1[0]), /* 1 */
	asn_DEF_UpdateLocationRes_tags_1,	/* Same as above */
	sizeof(asn_DEF_UpdateLocationRes_tags_1)
		/sizeof(asn_DEF_UpdateLocationRes_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_UpdateLocationRes_1,
	3,	/* Elements count */
	&asn_SPC_UpdateLocationRes_specs_1	/* Additional specs */
};

