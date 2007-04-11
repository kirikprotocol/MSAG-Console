#include <asn_internal.h>

#include "MessageType.h"

static asn_TYPE_member_t asn_MBR_MessageType_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.undirectional),
		(ASN_TAG_CLASS_APPLICATION | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Undirectional,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"undirectional"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.begin),
		(ASN_TAG_CLASS_APPLICATION | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Begin,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"begin"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.end),
		(ASN_TAG_CLASS_APPLICATION | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_End,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"end"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.contiinue),
		(ASN_TAG_CLASS_APPLICATION | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Continue,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"contiinue"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.abort),
		(ASN_TAG_CLASS_APPLICATION | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Abort,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"abort"
		},
};
static asn_TYPE_tag2member_t asn_MAP_MessageType_tag2el_1[] = {
    { (ASN_TAG_CLASS_APPLICATION | (1 << 2)), 0, 0, 0 }, /* undirectional at 19 */
    { (ASN_TAG_CLASS_APPLICATION | (2 << 2)), 1, 0, 0 }, /* begin at 20 */
    { (ASN_TAG_CLASS_APPLICATION | (4 << 2)), 2, 0, 0 }, /* end at 21 */
    { (ASN_TAG_CLASS_APPLICATION | (5 << 2)), 3, 0, 0 }, /* contiinue at 22 */
    { (ASN_TAG_CLASS_APPLICATION | (7 << 2)), 4, 0, 0 } /* abort at 24 */
};
static asn_CHOICE_specifics_t asn_SPC_MessageType_specs_1 = {
	sizeof(struct MessageType),
	offsetof(struct MessageType, _asn_ctx),
	offsetof(struct MessageType, present),
	sizeof(((struct MessageType *)0)->present),
	asn_MAP_MessageType_tag2el_1,
	5,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_MessageType = {
	"MessageType",
	"MessageType",
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
	asn_MBR_MessageType_1,
	5,	/* Elements count */
	&asn_SPC_MessageType_specs_1	/* Additional specs */
};

