#include <asn_internal.h>

#include "SMSEvent.h"

static asn_TYPE_member_t asn_MBR_SMSEvent_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SMSEvent, eventTypeSMS),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		(void *)&asn_DEF_EventTypeSMS,
		0,	/* Defer constraints checking to the member type */
		"eventTypeSMS"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SMSEvent, monitorMode),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		(void *)&asn_DEF_MonitorMode,
		0,	/* Defer constraints checking to the member type */
		"monitorMode"
		},
};
static ber_tlv_tag_t asn_DEF_SMSEvent_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SMSEvent_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* eventTypeSMS at 495 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* monitorMode at 497 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SMSEvent_1_specs = {
	sizeof(struct SMSEvent),
	offsetof(struct SMSEvent, _asn_ctx),
	asn_MAP_SMSEvent_1_tag2el,
	2,	/* Count of tags in the map */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SMSEvent = {
	"SMSEvent",
	"SMSEvent",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SMSEvent_1_tags,
	sizeof(asn_DEF_SMSEvent_1_tags)
		/sizeof(asn_DEF_SMSEvent_1_tags[0]), /* 1 */
	asn_DEF_SMSEvent_1_tags,	/* Same as above */
	sizeof(asn_DEF_SMSEvent_1_tags)
		/sizeof(asn_DEF_SMSEvent_1_tags[0]), /* 1 */
	asn_MBR_SMSEvent_1,
	2,	/* Elements count */
	&asn_SPC_SMSEvent_1_specs	/* Additional specs */
};

