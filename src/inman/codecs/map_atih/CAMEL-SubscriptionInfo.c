#ident "$Id$"

#include <asn_internal.h>

#include "CAMEL-SubscriptionInfo.h"

static asn_TYPE_member_t asn_MBR_CAMEL_SubscriptionInfo_1[] = {
	{ ATF_POINTER, 23, offsetof(struct CAMEL_SubscriptionInfo, o_CSI),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_O_CSI,
		0,	/* Defer constraints checking to the member type */
		"o-CSI"
		},
	{ ATF_POINTER, 22, offsetof(struct CAMEL_SubscriptionInfo, o_BcsmCamelTDP_CriteriaList),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_O_BcsmCamelTDPCriteriaList,
		0,	/* Defer constraints checking to the member type */
		"o-BcsmCamelTDP-CriteriaList"
		},
	{ ATF_POINTER, 21, offsetof(struct CAMEL_SubscriptionInfo, d_CSI),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_D_CSI,
		0,	/* Defer constraints checking to the member type */
		"d-CSI"
		},
	{ ATF_POINTER, 20, offsetof(struct CAMEL_SubscriptionInfo, t_CSI),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_CSI,
		0,	/* Defer constraints checking to the member type */
		"t-CSI"
		},
	{ ATF_POINTER, 19, offsetof(struct CAMEL_SubscriptionInfo, t_BCSM_CAMEL_TDP_CriteriaList),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList,
		0,	/* Defer constraints checking to the member type */
		"t-BCSM-CAMEL-TDP-CriteriaList"
		},
	{ ATF_POINTER, 18, offsetof(struct CAMEL_SubscriptionInfo, vt_CSI),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_CSI,
		0,	/* Defer constraints checking to the member type */
		"vt-CSI"
		},
	{ ATF_POINTER, 17, offsetof(struct CAMEL_SubscriptionInfo, vt_BCSM_CAMEL_TDP_CriteriaList),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList,
		0,	/* Defer constraints checking to the member type */
		"vt-BCSM-CAMEL-TDP-CriteriaList"
		},
	{ ATF_POINTER, 16, offsetof(struct CAMEL_SubscriptionInfo, tif_CSI),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"tif-CSI"
		},
	{ ATF_POINTER, 15, offsetof(struct CAMEL_SubscriptionInfo, tif_CSI_NotificationToCSE),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"tif-CSI-NotificationToCSE"
		},
	{ ATF_POINTER, 14, offsetof(struct CAMEL_SubscriptionInfo, gprs_CSI),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GPRS_CSI,
		0,	/* Defer constraints checking to the member type */
		"gprs-CSI"
		},
	{ ATF_POINTER, 13, offsetof(struct CAMEL_SubscriptionInfo, mo_sms_CSI),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SMS_CSI,
		0,	/* Defer constraints checking to the member type */
		"mo-sms-CSI"
		},
	{ ATF_POINTER, 12, offsetof(struct CAMEL_SubscriptionInfo, ss_CSI),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SS_CSI,
		0,	/* Defer constraints checking to the member type */
		"ss-CSI"
		},
	{ ATF_POINTER, 11, offsetof(struct CAMEL_SubscriptionInfo, m_CSI),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_M_CSI,
		0,	/* Defer constraints checking to the member type */
		"m-CSI"
		},
	{ ATF_POINTER, 10, offsetof(struct CAMEL_SubscriptionInfo, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 9, offsetof(struct CAMEL_SubscriptionInfo, specificCSIDeletedList),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SpecificCSI_Withdraw,
		0,	/* Defer constraints checking to the member type */
		"specificCSIDeletedList"
		},
	{ ATF_POINTER, 8, offsetof(struct CAMEL_SubscriptionInfo, mt_sms_CSI),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SMS_CSI,
		0,	/* Defer constraints checking to the member type */
		"mt-sms-CSI"
		},
	{ ATF_POINTER, 7, offsetof(struct CAMEL_SubscriptionInfo, mt_smsCAMELTDP_CriteriaList),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MT_smsCAMELTDP_CriteriaList,
		0,	/* Defer constraints checking to the member type */
		"mt-smsCAMELTDP-CriteriaList"
		},
	{ ATF_POINTER, 6, offsetof(struct CAMEL_SubscriptionInfo, mg_csi),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MG_CSI,
		0,	/* Defer constraints checking to the member type */
		"mg-csi"
		},
	{ ATF_POINTER, 5, offsetof(struct CAMEL_SubscriptionInfo, o_IM_CSI),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_O_CSI,
		0,	/* Defer constraints checking to the member type */
		"o-IM-CSI"
		},
	{ ATF_POINTER, 4, offsetof(struct CAMEL_SubscriptionInfo, o_IM_BcsmCamelTDP_CriteriaList),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_O_BcsmCamelTDPCriteriaList,
		0,	/* Defer constraints checking to the member type */
		"o-IM-BcsmCamelTDP-CriteriaList"
		},
	{ ATF_POINTER, 3, offsetof(struct CAMEL_SubscriptionInfo, d_IM_CSI),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_D_CSI,
		0,	/* Defer constraints checking to the member type */
		"d-IM-CSI"
		},
	{ ATF_POINTER, 2, offsetof(struct CAMEL_SubscriptionInfo, vt_IM_CSI),
		(ASN_TAG_CLASS_CONTEXT | (21 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_CSI,
		0,	/* Defer constraints checking to the member type */
		"vt-IM-CSI"
		},
	{ ATF_POINTER, 1, offsetof(struct CAMEL_SubscriptionInfo, vt_IM_BCSM_CAMEL_TDP_CriteriaList),
		(ASN_TAG_CLASS_CONTEXT | (22 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList,
		0,	/* Defer constraints checking to the member type */
		"vt-IM-BCSM-CAMEL-TDP-CriteriaList"
		},
};
static ber_tlv_tag_t asn_DEF_CAMEL_SubscriptionInfo_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CAMEL_SubscriptionInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* o-CSI at 834 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* o-BcsmCamelTDP-CriteriaList at 835 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* d-CSI at 836 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* t-CSI at 837 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* t-BCSM-CAMEL-TDP-CriteriaList at 838 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* vt-CSI at 839 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* vt-BCSM-CAMEL-TDP-CriteriaList at 840 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* tif-CSI at 841 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* tif-CSI-NotificationToCSE at 842 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* gprs-CSI at 843 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* mo-sms-CSI at 844 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* ss-CSI at 845 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* m-CSI at 846 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* extensionContainer at 847 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* specificCSIDeletedList at 849 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* mt-sms-CSI at 850 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* mt-smsCAMELTDP-CriteriaList at 851 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* mg-csi at 852 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* o-IM-CSI at 853 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* o-IM-BcsmCamelTDP-CriteriaList at 854 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 }, /* d-IM-CSI at 855 */
    { (ASN_TAG_CLASS_CONTEXT | (21 << 2)), 21, 0, 0 }, /* vt-IM-CSI at 856 */
    { (ASN_TAG_CLASS_CONTEXT | (22 << 2)), 22, 0, 0 } /* vt-IM-BCSM-CAMEL-TDP-CriteriaList at 857 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CAMEL_SubscriptionInfo_1_specs = {
	sizeof(struct CAMEL_SubscriptionInfo),
	offsetof(struct CAMEL_SubscriptionInfo, _asn_ctx),
	asn_MAP_CAMEL_SubscriptionInfo_1_tag2el,
	23,	/* Count of tags in the map */
	13,	/* Start extensions */
	24	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CAMEL_SubscriptionInfo = {
	"CAMEL-SubscriptionInfo",
	"CAMEL-SubscriptionInfo",
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
	asn_DEF_CAMEL_SubscriptionInfo_1_tags,
	sizeof(asn_DEF_CAMEL_SubscriptionInfo_1_tags)
		/sizeof(asn_DEF_CAMEL_SubscriptionInfo_1_tags[0]), /* 1 */
	asn_DEF_CAMEL_SubscriptionInfo_1_tags,	/* Same as above */
	sizeof(asn_DEF_CAMEL_SubscriptionInfo_1_tags)
		/sizeof(asn_DEF_CAMEL_SubscriptionInfo_1_tags[0]), /* 1 */
	asn_MBR_CAMEL_SubscriptionInfo_1,
	23,	/* Elements count */
	&asn_SPC_CAMEL_SubscriptionInfo_1_specs	/* Additional specs */
};

