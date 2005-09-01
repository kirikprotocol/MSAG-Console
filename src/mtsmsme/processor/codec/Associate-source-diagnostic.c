#include <asn_internal.h>

#include <Associate-source-diagnostic.h>

static asn_TYPE_member_t asn_MBR_Associate_source_diagnostic[] = {
  { ATF_NOFLAGS, 0, offsetof(struct Associate_source_diagnostic, choice.dialogue_service_user),
    (ASN_TAG_CLASS_CONTEXT | (1 << 2)),
    +1, /* EXPLICIT tag at current level */
    (void *)&asn_DEF_NativeInteger,
    0,  /* Defer constraints checking to the member type */
    "dialogue-service-user"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Associate_source_diagnostic, choice.dialogue_service_provider),
    (ASN_TAG_CLASS_CONTEXT | (2 << 2)),
    +1, /* EXPLICIT tag at current level */
    (void *)&asn_DEF_NativeInteger,
    0,  /* Defer constraints checking to the member type */
    "dialogue-service-provider"
    },
};
static asn_TYPE_tag2member_t asn_DEF_Associate_source_diagnostic_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* dialogue-service-user at 57 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* dialogue-service-provider at 60 */
};
static asn_CHOICE_specifics_t asn_DEF_Associate_source_diagnostic_specs = {
  sizeof(struct Associate_source_diagnostic),
  offsetof(struct Associate_source_diagnostic, _asn_ctx),
  offsetof(struct Associate_source_diagnostic, present),
  sizeof(((struct Associate_source_diagnostic *)0)->present),
  asn_DEF_Associate_source_diagnostic_tag2el,
  2,  /* Count of tags in the map */
  0 /* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_Associate_source_diagnostic = {
  "Associate-source-diagnostic",
  "Associate-source-diagnostic",
  CHOICE_free,
  CHOICE_print,
  CHOICE_constraint,
  CHOICE_decode_ber,
  CHOICE_encode_der,
  CHOICE_decode_xer,
  CHOICE_encode_xer,
  CHOICE_outmost_tag,
  0,  /* No effective tags (pointer) */
  0,  /* No effective tags (count) */
  0,  /* No tags (pointer) */
  0,  /* No tags (count) */
  asn_MBR_Associate_source_diagnostic,
  2,  /* Elements count */
  &asn_DEF_Associate_source_diagnostic_specs  /* Additional specs */
};
