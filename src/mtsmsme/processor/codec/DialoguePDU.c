#include <asn_internal.h>

#include "DialoguePDU.h"

static asn_TYPE_member_t asn_MBR_DialoguePDU_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DialoguePDU, choice.dialogueRequest),
		(ASN_TAG_CLASS_APPLICATION | (0 << 2)),
		0,
		&asn_DEF_AARQ_apdu,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dialogueRequest"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DialoguePDU, choice.dialogueResponse),
		(ASN_TAG_CLASS_APPLICATION | (1 << 2)),
		0,
		&asn_DEF_AARE_apdu,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dialogueResponse"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DialoguePDU, choice.dialogueAbort),
		(ASN_TAG_CLASS_APPLICATION | (4 << 2)),
		0,
		&asn_DEF_ABRT_apdu,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dialogueAbort"
		},
};
static asn_TYPE_tag2member_t asn_MAP_DialoguePDU_tag2el_1[] = {
    { (ASN_TAG_CLASS_APPLICATION | (0 << 2)), 0, 0, 0 }, /* dialogueRequest at 13 */
    { (ASN_TAG_CLASS_APPLICATION | (1 << 2)), 1, 0, 0 }, /* dialogueResponse at 14 */
    { (ASN_TAG_CLASS_APPLICATION | (4 << 2)), 2, 0, 0 } /* dialogueAbort at 16 */
};
static asn_CHOICE_specifics_t asn_SPC_DialoguePDU_specs_1 = {
	sizeof(struct DialoguePDU),
	offsetof(struct DialoguePDU, _asn_ctx),
	offsetof(struct DialoguePDU, present),
	sizeof(((struct DialoguePDU *)0)->present),
	asn_MAP_DialoguePDU_tag2el_1,
	3,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_DialoguePDU = {
	"DialoguePDU",
	"DialoguePDU",
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
	asn_MBR_DialoguePDU_1,
	3,	/* Elements count */
	&asn_SPC_DialoguePDU_specs_1	/* Additional specs */
};

