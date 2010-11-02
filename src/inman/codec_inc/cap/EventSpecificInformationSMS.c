#ident "$Id$"

#include <asn_internal.h>

#include "EventSpecificInformationSMS.h"

static asn_TYPE_member_t asn_MBR_o_smsFailureSpecificInfo_2[] = {
	{ ATF_POINTER, 1, offsetof(o_smsFailureSpecificInfo_t, failureCause),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MO_SMSCause,
		0,	/* Defer constraints checking to the member type */
		"failureCause"
		},
};
static ber_tlv_tag_t asn_DEF_o_smsFailureSpecificInfo_2_tags[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_o_smsFailureSpecificInfo_2_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* failureCause at 116 */
};
static asn_SEQUENCE_specifics_t asn_SPC_o_smsFailureSpecificInfo_2_specs = {
	sizeof(o_smsFailureSpecificInfo_t),
	offsetof(o_smsFailureSpecificInfo_t, _asn_ctx),
	asn_MAP_o_smsFailureSpecificInfo_2_tag2el,
	1,	/* Count of tags in the map */
	0,	/* Start extensions */
	2	/* Stop extensions */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_o_smsFailureSpecificInfo_2 = {
	"o-smsFailureSpecificInfo",
	"o-smsFailureSpecificInfo",
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
	asn_DEF_o_smsFailureSpecificInfo_2_tags,
	sizeof(asn_DEF_o_smsFailureSpecificInfo_2_tags)
		/sizeof(asn_DEF_o_smsFailureSpecificInfo_2_tags[0]) - 1, /* 1 */
	asn_DEF_o_smsFailureSpecificInfo_2_tags,	/* Same as above */
	sizeof(asn_DEF_o_smsFailureSpecificInfo_2_tags)
		/sizeof(asn_DEF_o_smsFailureSpecificInfo_2_tags[0]), /* 2 */
	asn_MBR_o_smsFailureSpecificInfo_2,
	1,	/* Elements count */
	&asn_SPC_o_smsFailureSpecificInfo_2_specs	/* Additional specs */
};

static ber_tlv_tag_t asn_DEF_o_smsSubmissionSpecificInfo_5_tags[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SEQUENCE_specifics_t asn_SPC_o_smsSubmissionSpecificInfo_5_specs = {
	sizeof(o_smsSubmissionSpecificInfo_t),
	offsetof(o_smsSubmissionSpecificInfo_t, _asn_ctx),
	0,	/* No top level tags */
	0,	/* No tags in the map */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_o_smsSubmissionSpecificInfo_5 = {
	"o-smsSubmissionSpecificInfo",
	"o-smsSubmissionSpecificInfo",
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
	asn_DEF_o_smsSubmissionSpecificInfo_5_tags,
	sizeof(asn_DEF_o_smsSubmissionSpecificInfo_5_tags)
		/sizeof(asn_DEF_o_smsSubmissionSpecificInfo_5_tags[0]) - 1, /* 1 */
	asn_DEF_o_smsSubmissionSpecificInfo_5_tags,	/* Same as above */
	sizeof(asn_DEF_o_smsSubmissionSpecificInfo_5_tags)
		/sizeof(asn_DEF_o_smsSubmissionSpecificInfo_5_tags[0]), /* 2 */
	0, 0,	/* No members */
	&asn_SPC_o_smsSubmissionSpecificInfo_5_specs	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_t_smsFailureSpecificInfo_7[] = {
	{ ATF_POINTER, 1, offsetof(t_smsFailureSpecificInfo_t, failureCause),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MT_SMSCause,
		0,	/* Defer constraints checking to the member type */
		"failureCause"
		},
};
static ber_tlv_tag_t asn_DEF_t_smsFailureSpecificInfo_7_tags[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_t_smsFailureSpecificInfo_7_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* failureCause at 123 */
};
static asn_SEQUENCE_specifics_t asn_SPC_t_smsFailureSpecificInfo_7_specs = {
	sizeof(t_smsFailureSpecificInfo_t),
	offsetof(t_smsFailureSpecificInfo_t, _asn_ctx),
	asn_MAP_t_smsFailureSpecificInfo_7_tag2el,
	1,	/* Count of tags in the map */
	0,	/* Start extensions */
	2	/* Stop extensions */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_t_smsFailureSpecificInfo_7 = {
	"t-smsFailureSpecificInfo",
	"t-smsFailureSpecificInfo",
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
	asn_DEF_t_smsFailureSpecificInfo_7_tags,
	sizeof(asn_DEF_t_smsFailureSpecificInfo_7_tags)
		/sizeof(asn_DEF_t_smsFailureSpecificInfo_7_tags[0]) - 1, /* 1 */
	asn_DEF_t_smsFailureSpecificInfo_7_tags,	/* Same as above */
	sizeof(asn_DEF_t_smsFailureSpecificInfo_7_tags)
		/sizeof(asn_DEF_t_smsFailureSpecificInfo_7_tags[0]), /* 2 */
	asn_MBR_t_smsFailureSpecificInfo_7,
	1,	/* Elements count */
	&asn_SPC_t_smsFailureSpecificInfo_7_specs	/* Additional specs */
};

static ber_tlv_tag_t asn_DEF_t_smsDeliverySpecificInfo_10_tags[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SEQUENCE_specifics_t asn_SPC_t_smsDeliverySpecificInfo_10_specs = {
	sizeof(t_smsDeliverySpecificInfo_t),
	offsetof(t_smsDeliverySpecificInfo_t, _asn_ctx),
	0,	/* No top level tags */
	0,	/* No tags in the map */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_t_smsDeliverySpecificInfo_10 = {
	"t-smsDeliverySpecificInfo",
	"t-smsDeliverySpecificInfo",
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
	asn_DEF_t_smsDeliverySpecificInfo_10_tags,
	sizeof(asn_DEF_t_smsDeliverySpecificInfo_10_tags)
		/sizeof(asn_DEF_t_smsDeliverySpecificInfo_10_tags[0]) - 1, /* 1 */
	asn_DEF_t_smsDeliverySpecificInfo_10_tags,	/* Same as above */
	sizeof(asn_DEF_t_smsDeliverySpecificInfo_10_tags)
		/sizeof(asn_DEF_t_smsDeliverySpecificInfo_10_tags[0]), /* 2 */
	0, 0,	/* No members */
	&asn_SPC_t_smsDeliverySpecificInfo_10_specs	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_EventSpecificInformationSMS_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct EventSpecificInformationSMS, choice.o_smsFailureSpecificInfo),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_o_smsFailureSpecificInfo_2,
		0,	/* Defer constraints checking to the member type */
		"o-smsFailureSpecificInfo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct EventSpecificInformationSMS, choice.o_smsSubmissionSpecificInfo),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_o_smsSubmissionSpecificInfo_5,
		0,	/* Defer constraints checking to the member type */
		"o-smsSubmissionSpecificInfo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct EventSpecificInformationSMS, choice.t_smsFailureSpecificInfo),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_t_smsFailureSpecificInfo_7,
		0,	/* Defer constraints checking to the member type */
		"t-smsFailureSpecificInfo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct EventSpecificInformationSMS, choice.t_smsDeliverySpecificInfo),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_t_smsDeliverySpecificInfo_10,
		0,	/* Defer constraints checking to the member type */
		"t-smsDeliverySpecificInfo"
		},
};
static asn_TYPE_tag2member_t asn_MAP_EventSpecificInformationSMS_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* o-smsFailureSpecificInfo at 116 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* o-smsSubmissionSpecificInfo at 121 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* t-smsFailureSpecificInfo at 123 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* t-smsDeliverySpecificInfo at 128 */
};
static asn_CHOICE_specifics_t asn_SPC_EventSpecificInformationSMS_1_specs = {
	sizeof(struct EventSpecificInformationSMS),
	offsetof(struct EventSpecificInformationSMS, _asn_ctx),
	offsetof(struct EventSpecificInformationSMS, present),
	sizeof(((struct EventSpecificInformationSMS *)0)->present),
	asn_MAP_EventSpecificInformationSMS_1_tag2el,
	4,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_EventSpecificInformationSMS = {
	"EventSpecificInformationSMS",
	"EventSpecificInformationSMS",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
#ifndef ASN1_XER_NOT_USED
	CHOICE_decode_xer,
	CHOICE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	asn_MBR_EventSpecificInformationSMS_1,
	4,	/* Elements count */
	&asn_SPC_EventSpecificInformationSMS_1_specs	/* Additional specs */
};

