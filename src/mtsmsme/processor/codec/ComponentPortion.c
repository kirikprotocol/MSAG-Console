#include <asn_internal.h>

#include <ComponentPortion.h>

static asn_TYPE_member_t asn_MBR_ComponentPortion[] = {
  { ATF_NOFLAGS, 0, 0,
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_Component,
    0,  /* Defer constraints checking to the member type */
    ""
    },
};
static ber_tlv_tag_t asn_DEF_ComponentPortion_tags[] = {
  (ASN_TAG_CLASS_APPLICATION | (12 << 2)),
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_DEF_ComponentPortion_specs = {
  sizeof(struct ComponentPortion),
  offsetof(struct ComponentPortion, _asn_ctx),
  1,  /* XER encoding is XMLValueList */
};
asn_TYPE_descriptor_t asn_DEF_ComponentPortion = {
  "ComponentPortion",
  "ComponentPortion",
  SEQUENCE_OF_free,
  SEQUENCE_OF_print,
  SEQUENCE_OF_constraint,
  SEQUENCE_OF_decode_ber,
  SEQUENCE_OF_encode_der,
  0,        /* Not implemented yet */
  SEQUENCE_OF_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_ComponentPortion_tags,
  sizeof(asn_DEF_ComponentPortion_tags)
    /sizeof(asn_DEF_ComponentPortion_tags[0]) - 1, /* 1 */
  asn_DEF_ComponentPortion_tags,  /* Same as above */
  sizeof(asn_DEF_ComponentPortion_tags)
    /sizeof(asn_DEF_ComponentPortion_tags[0]), /* 2 */
  asn_MBR_ComponentPortion,
  1,  /* Single element */
  &asn_DEF_ComponentPortion_specs /* Additional specs */
};
