#include <asn_internal.h>

#include "Code.h"

static asn_TYPE_member_t asn_MBR_Code_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Code, choice.local),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"local"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Code, choice.global),
		(ASN_TAG_CLASS_UNIVERSAL | (6 << 2)),
		0,
		&asn_DEF_OBJECT_IDENTIFIER,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"global"
		},
};
static asn_TYPE_tag2member_t asn_MAP_Code_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* local at 54 */
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 1, 0, 0 } /* global at 55 */
};
static asn_CHOICE_specifics_t asn_SPC_Code_specs_1 = {
	sizeof(struct Code),
	offsetof(struct Code, _asn_ctx),
	offsetof(struct Code, present),
	sizeof(((struct Code *)0)->present),
	asn_MAP_Code_tag2el_1,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_Code = {
	"Code",
	"Code",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	asn_MBR_Code_1,
	2,	/* Elements count */
	&asn_SPC_Code_specs_1	/* Additional specs */
};

