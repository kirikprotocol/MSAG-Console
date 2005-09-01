#include <asn_internal.h>

#include <AARE-apdu.h>

static asn_TYPE_member_t asn_MBR_aare_user_information[] = {
  { ATF_NOFLAGS, 0, 0,
    (ASN_TAG_CLASS_UNIVERSAL | (8 << 2)),
    0,
    (void *)&asn_DEF_MEXT,
    0,  /* Defer constraints checking to the member type */
    ""
    },
};
static ber_tlv_tag_t asn_DEF_aare_user_information_tags[] = {
  (ASN_TAG_CLASS_CONTEXT | (30 << 2)),
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_DEF_aare_user_information_specs = {
  sizeof(struct aare_user_information),
  offsetof(struct aare_user_information, _asn_ctx),
  0,  /* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_aare_user_information = {
  "aare-user-information",
  "aare-user-information",
  SEQUENCE_OF_free,
  SEQUENCE_OF_print,
  SEQUENCE_OF_constraint,
  SEQUENCE_OF_decode_ber,
  SEQUENCE_OF_encode_der,
  0,        /* Not implemented yet */
  SEQUENCE_OF_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_aare_user_information_tags,
  sizeof(asn_DEF_aare_user_information_tags)
    /sizeof(asn_DEF_aare_user_information_tags[0]) - 1, /* 1 */
  asn_DEF_aare_user_information_tags, /* Same as above */
  sizeof(asn_DEF_aare_user_information_tags)
    /sizeof(asn_DEF_aare_user_information_tags[0]), /* 2 */
  asn_MBR_aare_user_information,
  1,  /* Single element */
  &asn_DEF_aare_user_information_specs  /* Additional specs */
};

static asn_TYPE_member_t asn_MBR_AARE_apdu[] = {
  { ATF_POINTER, 1, offsetof(struct AARE_apdu, protocol_version),
    (ASN_TAG_CLASS_CONTEXT | (0 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_BIT_STRING,
    0,  /* Defer constraints checking to the member type */
    "protocol-version"
    },
  { ATF_NOFLAGS, 0, offsetof(struct AARE_apdu, application_context_name),
    (ASN_TAG_CLASS_CONTEXT | (1 << 2)),
    +1, /* EXPLICIT tag at current level */
    (void *)&asn_DEF_OBJECT_IDENTIFIER,
    0,  /* Defer constraints checking to the member type */
    "application-context-name"
    },
  { ATF_NOFLAGS, 0, offsetof(struct AARE_apdu, result),
    (ASN_TAG_CLASS_CONTEXT | (2 << 2)),
    +1, /* EXPLICIT tag at current level */
    (void *)&asn_DEF_Associate_result,
    0,  /* Defer constraints checking to the member type */
    "result"
    },
  { ATF_NOFLAGS, 0, offsetof(struct AARE_apdu, result_source_diagnostic),
    (ASN_TAG_CLASS_CONTEXT | (3 << 2)),
    +1, /* EXPLICIT tag at current level */
    (void *)&asn_DEF_Associate_source_diagnostic,
    0,  /* Defer constraints checking to the member type */
    "result-source-diagnostic"
    },
  { ATF_POINTER, 1, offsetof(struct AARE_apdu, aare_user_information),
    (ASN_TAG_CLASS_CONTEXT | (30 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_aare_user_information,
    0,  /* Defer constraints checking to the member type */
    "aare-user-information"
    },
};
static ber_tlv_tag_t asn_DEF_AARE_apdu_tags[] = {
  (ASN_TAG_CLASS_APPLICATION | (1 << 2)),
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_AARE_apdu_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* protocol-version at 38 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* application-context-name at 39 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* result at 40 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* result-source-diagnostic at 41 */
    { (ASN_TAG_CLASS_CONTEXT | (30 << 2)), 4, 0, 0 }, /* aare-user-information at 42 */
};
static asn_SEQUENCE_specifics_t asn_DEF_AARE_apdu_specs = {
  sizeof(struct AARE_apdu),
  offsetof(struct AARE_apdu, _asn_ctx),
  asn_DEF_AARE_apdu_tag2el,
  5,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_AARE_apdu = {
  "AARE-apdu",
  "AARE-apdu",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_AARE_apdu_tags,
  sizeof(asn_DEF_AARE_apdu_tags)
    /sizeof(asn_DEF_AARE_apdu_tags[0]) - 1, /* 1 */
  asn_DEF_AARE_apdu_tags, /* Same as above */
  sizeof(asn_DEF_AARE_apdu_tags)
    /sizeof(asn_DEF_AARE_apdu_tags[0]), /* 2 */
  asn_MBR_AARE_apdu,
  5,  /* Elements count */
  &asn_DEF_AARE_apdu_specs  /* Additional specs */
};
