#include <asn_internal.h>

#include <ReturnResult.h>

static asn_TYPE_member_t asn_MBR_result[] = {
  { ATF_NOFLAGS, 0, offsetof(struct result, operationCode),
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_NULL,
    0,  /* Defer constraints checking to the member type */
    "operationCode"
    },
  { ATF_OPEN_TYPE | ATF_NOFLAGS, 0, offsetof(struct result, parameter),
    -1 /* Ambiguous tag (ANY?) */,
    0,
    (void *)&asn_DEF_ANY,
    0,  /* Defer constraints checking to the member type */
    "parameter"
    },
};
static ber_tlv_tag_t asn_DEF_result_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_result_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* local at 51 */
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 0, 0, 0 }, /* global at 52 */
};
static asn_SEQUENCE_specifics_t asn_DEF_result_specs = {
  sizeof(struct result),
  offsetof(struct result, _asn_ctx),
  asn_DEF_result_tag2el,
  2,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_result = {
  "result",
  "result",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_result_tags,
  sizeof(asn_DEF_result_tags)
    /sizeof(asn_DEF_result_tags[0]), /* 1 */
  asn_DEF_result_tags,  /* Same as above */
  sizeof(asn_DEF_result_tags)
    /sizeof(asn_DEF_result_tags[0]), /* 1 */
  asn_MBR_result,
  2,  /* Elements count */
  &asn_DEF_result_specs /* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ReturnResult[] = {
  { ATF_NOFLAGS, 0, offsetof(struct ReturnResult, invokeID),
    (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
    0,
    (void *)&asn_DEF_InvokeIdType,
    0,  /* Defer constraints checking to the member type */
    "invokeID"
    },
  { ATF_POINTER, 1, offsetof(struct ReturnResult, result),
    (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
    0,
    (void *)&asn_DEF_result,
    0,  /* Defer constraints checking to the member type */
    "result"
    },
};
static ber_tlv_tag_t asn_DEF_ReturnResult_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_ReturnResult_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* invokeID at 119 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 }, /* result at 122 */
};
static asn_SEQUENCE_specifics_t asn_DEF_ReturnResult_specs = {
  sizeof(struct ReturnResult),
  offsetof(struct ReturnResult, _asn_ctx),
  asn_DEF_ReturnResult_tag2el,
  2,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ReturnResult = {
  "ReturnResult",
  "ReturnResult",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_ReturnResult_tags,
  sizeof(asn_DEF_ReturnResult_tags)
    /sizeof(asn_DEF_ReturnResult_tags[0]), /* 1 */
  asn_DEF_ReturnResult_tags,  /* Same as above */
  sizeof(asn_DEF_ReturnResult_tags)
    /sizeof(asn_DEF_ReturnResult_tags[0]), /* 1 */
  asn_MBR_ReturnResult,
  2,  /* Elements count */
  &asn_DEF_ReturnResult_specs /* Additional specs */
};
