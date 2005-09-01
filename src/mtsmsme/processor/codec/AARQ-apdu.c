#include <asn_internal.h>

#include <AARQ-apdu.h>

static asn_TYPE_member_t asn_MBR_aarq_user_information[] = {
  { ATF_NOFLAGS, 0, 0,
    (ASN_TAG_CLASS_UNIVERSAL | (8 << 2)),
    0,
    (void *)&asn_DEF_MEXT,
    0,  /* Defer constraints checking to the member type */
    ""
    },
};
static ber_tlv_tag_t asn_DEF_aarq_user_information_tags[] = {
  (ASN_TAG_CLASS_CONTEXT | (30 << 2)),
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_DEF_aarq_user_information_specs = {
  sizeof(struct aarq_user_information),
  offsetof(struct aarq_user_information, _asn_ctx),
  0,  /* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_aarq_user_information = {
  "aarq-user-information",
  "aarq-user-information",
  SEQUENCE_OF_free,
  SEQUENCE_OF_print,
  SEQUENCE_OF_constraint,
  SEQUENCE_OF_decode_ber,
  SEQUENCE_OF_encode_der,
  0,        /* Not implemented yet */
  SEQUENCE_OF_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_aarq_user_information_tags,
  sizeof(asn_DEF_aarq_user_information_tags)
    /sizeof(asn_DEF_aarq_user_information_tags[0]) - 1, /* 1 */
  asn_DEF_aarq_user_information_tags, /* Same as above */
  sizeof(asn_DEF_aarq_user_information_tags)
    /sizeof(asn_DEF_aarq_user_information_tags[0]), /* 2 */
  asn_MBR_aarq_user_information,
  1,  /* Single element */
  &asn_DEF_aarq_user_information_specs  /* Additional specs */
};

static asn_TYPE_member_t asn_MBR_AARQ_apdu[] = {
  { ATF_POINTER, 1, offsetof(struct AARQ_apdu, protocol_version),
    (ASN_TAG_CLASS_CONTEXT | (0 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_BIT_STRING,
    0,  /* Defer constraints checking to the member type */
    "protocol-version"
    },
  { ATF_NOFLAGS, 0, offsetof(struct AARQ_apdu, application_context_name),
    (ASN_TAG_CLASS_CONTEXT | (1 << 2)),
    +1, /* EXPLICIT tag at current level */
    (void *)&asn_DEF_OBJECT_IDENTIFIER,
    0,  /* Defer constraints checking to the member type */
    "application-context-name"
    },
  { ATF_POINTER, 1, offsetof(struct AARQ_apdu, aarq_user_information),
    (ASN_TAG_CLASS_CONTEXT | (30 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_aarq_user_information,
    0,  /* Defer constraints checking to the member type */
    "aarq-user-information"
    },
};
static ber_tlv_tag_t asn_DEF_AARQ_apdu_tags[] = {
  (ASN_TAG_CLASS_APPLICATION | (0 << 2)),
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_AARQ_apdu_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* protocol-version at 31 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* application-context-name at 32 */
    { (ASN_TAG_CLASS_CONTEXT | (30 << 2)), 2, 0, 0 }, /* aarq-user-information at 33 */
};
static asn_SEQUENCE_specifics_t asn_DEF_AARQ_apdu_specs = {
  sizeof(struct AARQ_apdu),
  offsetof(struct AARQ_apdu, _asn_ctx),
  asn_DEF_AARQ_apdu_tag2el,
  3,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_AARQ_apdu = {
  "AARQ-apdu",
  "AARQ-apdu",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_AARQ_apdu_tags,
  sizeof(asn_DEF_AARQ_apdu_tags)
    /sizeof(asn_DEF_AARQ_apdu_tags[0]) - 1, /* 1 */
  asn_DEF_AARQ_apdu_tags, /* Same as above */
  sizeof(asn_DEF_AARQ_apdu_tags)
    /sizeof(asn_DEF_AARQ_apdu_tags[0]), /* 2 */
  asn_MBR_AARQ_apdu,
  3,  /* Elements count */
  &asn_DEF_AARQ_apdu_specs  /* Additional specs */
};
