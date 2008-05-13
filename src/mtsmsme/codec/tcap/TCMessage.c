#include <asn_internal.h>

#include "TCMessage.h"

static asn_TYPE_member_t asn_MBR_TCMessage_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct TCMessage, choice.begin),
		(ASN_TAG_CLASS_APPLICATION | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Begin,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"begin"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TCMessage, choice.end),
		(ASN_TAG_CLASS_APPLICATION | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_End,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"end"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TCMessage, choice.contiinue),
		(ASN_TAG_CLASS_APPLICATION | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Continue,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"contiinue"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TCMessage, choice.abort),
		(ASN_TAG_CLASS_APPLICATION | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Abort,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"abort"
		},
};
static asn_TYPE_tag2member_t asn_MAP_TCMessage_tag2el_1[] = {
    { (ASN_TAG_CLASS_APPLICATION | (2 << 2)), 0, 0, 0 }, /* begin at 22 */
    { (ASN_TAG_CLASS_APPLICATION | (4 << 2)), 1, 0, 0 }, /* end at 23 */
    { (ASN_TAG_CLASS_APPLICATION | (5 << 2)), 2, 0, 0 }, /* contiinue at 24 */
    { (ASN_TAG_CLASS_APPLICATION | (7 << 2)), 3, 0, 0 } /* abort at 26 */
};
static asn_CHOICE_specifics_t asn_SPC_TCMessage_specs_1 = {
	sizeof(struct TCMessage),
	offsetof(struct TCMessage, _asn_ctx),
	offsetof(struct TCMessage, present),
	sizeof(((struct TCMessage *)0)->present),
	asn_MAP_TCMessage_tag2el_1,
	4,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_TCMessage = {
	"TCMessage",
	"TCMessage",
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
	asn_MBR_TCMessage_1,
	4,	/* Elements count */
	&asn_SPC_TCMessage_specs_1	/* Additional specs */
};

