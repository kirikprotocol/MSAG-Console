#include <asn_internal.h>

#include <MO-forward.h>

static int
memb_imsi_1_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
      asn_app_consume_bytes_f *app_errlog, void *app_key) {
  const OCTET_STRING_t *st = sptr;
  size_t size;

  if(!sptr) {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: value not given (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }

  size = st->size;

  if((size >= 3 && size <= 8)) {
    /* Constraint check succeeded */
    return 0;
  } else {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: constraint failed (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }
}

static int
memb_lmsi_2_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
      asn_app_consume_bytes_f *app_errlog, void *app_key) {
  const OCTET_STRING_t *st = sptr;
  size_t size;

  if(!sptr) {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: value not given (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }

  size = st->size;

  if((size == 4)) {
    /* Constraint check succeeded */
    return 0;
  } else {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: constraint failed (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }
}

static int
memb_serviceCentreAddressDA_3_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
      asn_app_consume_bytes_f *app_errlog, void *app_key) {
  const OCTET_STRING_t *st = sptr;
  size_t size;

  if(!sptr) {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: value not given (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }

  size = st->size;

  if((size >= 1 && size <= 20)) {
    /* Constraint check succeeded */
    return 0;
  } else {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: constraint failed (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }
}

static int
memb_msisdn_4_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
      asn_app_consume_bytes_f *app_errlog, void *app_key) {
  const OCTET_STRING_t *st = sptr;
  size_t size;

  if(!sptr) {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: value not given (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }

  size = st->size;

  if((size >= 1 && size <= 9)) {
    /* Constraint check succeeded */
    return 0;
  } else {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: constraint failed (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }
}

static int
memb_serviceCentreAddressOA_5_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
      asn_app_consume_bytes_f *app_errlog, void *app_key) {
  const OCTET_STRING_t *st = sptr;
  size_t size;

  if(!sptr) {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: value not given (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }

  size = st->size;

  if((size >= 1 && size <= 20)) {
    /* Constraint check succeeded */
    return 0;
  } else {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: constraint failed (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }
}

static int
memb_sm_RP_UI_6_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
      asn_app_consume_bytes_f *app_errlog, void *app_key) {
  const OCTET_STRING_t *st = sptr;
  size_t size;

  if(!sptr) {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: value not given (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }

  size = st->size;

  if((size >= 1 && size <= 200)) {
    /* Constraint check succeeded */
    return 0;
  } else {
    _ASN_ERRLOG(app_errlog, app_key,
      "%s: constraint failed (%s:%d)",
      td->name, __FILE__, __LINE__);
    return -1;
  }
}

static asn_TYPE_member_t asn_MBR_sm_RP_DA[] = {
  { ATF_NOFLAGS, 0, offsetof(struct sm_RP_DA, choice.imsi),
    (ASN_TAG_CLASS_CONTEXT | (0 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_OCTET_STRING,
    memb_imsi_1_constraint,
    "imsi"
    },
  { ATF_NOFLAGS, 0, offsetof(struct sm_RP_DA, choice.lmsi),
    (ASN_TAG_CLASS_CONTEXT | (1 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_OCTET_STRING,
    memb_lmsi_2_constraint,
    "lmsi"
    },
  { ATF_NOFLAGS, 0, offsetof(struct sm_RP_DA, choice.serviceCentreAddressDA),
    (ASN_TAG_CLASS_CONTEXT | (4 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_OCTET_STRING,
    memb_serviceCentreAddressDA_3_constraint,
    "serviceCentreAddressDA"
    },
  { ATF_NOFLAGS, 0, offsetof(struct sm_RP_DA, choice.noSM_RP_DA),
    (ASN_TAG_CLASS_CONTEXT | (5 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_NULL,
    0,  /* Defer constraints checking to the member type */
    "noSM-RP-DA"
    },
};
static asn_TYPE_tag2member_t asn_DEF_sm_RP_DA_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* imsi at 187 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* lmsi at 188 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 2, 0, 0 }, /* serviceCentreAddressDA at 189 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 3, 0, 0 }, /* noSM-RP-DA at 190 */
};
static asn_CHOICE_specifics_t asn_DEF_sm_RP_DA_specs = {
  sizeof(struct sm_RP_DA),
  offsetof(struct sm_RP_DA, _asn_ctx),
  offsetof(struct sm_RP_DA, present),
  sizeof(((struct sm_RP_DA *)0)->present),
  asn_DEF_sm_RP_DA_tag2el,
  4,  /* Count of tags in the map */
  0 /* Whether extensible */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_sm_RP_DA = {
  "sm-RP-DA",
  "sm-RP-DA",
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
  asn_MBR_sm_RP_DA,
  4,  /* Elements count */
  &asn_DEF_sm_RP_DA_specs /* Additional specs */
};

static asn_TYPE_member_t asn_MBR_sm_RP_OA[] = {
  { ATF_NOFLAGS, 0, offsetof(struct sm_RP_OA, choice.msisdn),
    (ASN_TAG_CLASS_CONTEXT | (2 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_OCTET_STRING,
    memb_msisdn_4_constraint,
    "msisdn"
    },
  { ATF_NOFLAGS, 0, offsetof(struct sm_RP_OA, choice.serviceCentreAddressOA),
    (ASN_TAG_CLASS_CONTEXT | (4 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_OCTET_STRING,
    memb_serviceCentreAddressOA_5_constraint,
    "serviceCentreAddressOA"
    },
  { ATF_NOFLAGS, 0, offsetof(struct sm_RP_OA, choice.noSM_RP_OA),
    (ASN_TAG_CLASS_CONTEXT | (5 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_NULL,
    0,  /* Defer constraints checking to the member type */
    "noSM-RP-OA"
    },
};
static asn_TYPE_tag2member_t asn_DEF_sm_RP_OA_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* msisdn at 192 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 1, 0, 0 }, /* serviceCentreAddressOA at 193 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 2, 0, 0 }, /* noSM-RP-OA at 194 */
};
static asn_CHOICE_specifics_t asn_DEF_sm_RP_OA_specs = {
  sizeof(struct sm_RP_OA),
  offsetof(struct sm_RP_OA, _asn_ctx),
  offsetof(struct sm_RP_OA, present),
  sizeof(((struct sm_RP_OA *)0)->present),
  asn_DEF_sm_RP_OA_tag2el,
  3,  /* Count of tags in the map */
  0 /* Whether extensible */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_sm_RP_OA = {
  "sm-RP-OA",
  "sm-RP-OA",
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
  asn_MBR_sm_RP_OA,
  3,  /* Elements count */
  &asn_DEF_sm_RP_OA_specs /* Additional specs */
};

static asn_TYPE_member_t asn_MBR_MO_forward[] = {
  { ATF_NOFLAGS, 0, offsetof(struct MO_forward, sm_RP_DA),
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_sm_RP_DA,
    0,  /* Defer constraints checking to the member type */
    "sm-RP-DA"
    },
  { ATF_NOFLAGS, 0, offsetof(struct MO_forward, sm_RP_OA),
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_sm_RP_OA,
    0,  /* Defer constraints checking to the member type */
    "sm-RP-OA"
    },
  { ATF_NOFLAGS, 0, offsetof(struct MO_forward, sm_RP_UI),
    (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
    0,
    (void *)&asn_DEF_OCTET_STRING,
    memb_sm_RP_UI_6_constraint,
    "sm-RP-UI"
    },
};
static ber_tlv_tag_t asn_DEF_MO_forward_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_MO_forward_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 2, 0, 0 }, /* sm-RP-UI at 195 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* imsi at 187 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* lmsi at 188 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* msisdn at 192 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 0, 0, 1 }, /* serviceCentreAddressDA at 189 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 1, -1, 0 }, /* serviceCentreAddressOA at 193 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 0, 0, 1 }, /* noSM-RP-DA at 190 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 1, -1, 0 }, /* noSM-RP-OA at 194 */
};
static asn_SEQUENCE_specifics_t asn_DEF_MO_forward_specs = {
  sizeof(struct MO_forward),
  offsetof(struct MO_forward, _asn_ctx),
  asn_DEF_MO_forward_tag2el,
  8,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_MO_forward = {
  "MO-forward",
  "MO-forward",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_MO_forward_tags,
  sizeof(asn_DEF_MO_forward_tags)
    /sizeof(asn_DEF_MO_forward_tags[0]), /* 1 */
  asn_DEF_MO_forward_tags,  /* Same as above */
  sizeof(asn_DEF_MO_forward_tags)
    /sizeof(asn_DEF_MO_forward_tags[0]), /* 1 */
  asn_MBR_MO_forward,
  3,  /* Elements count */
  &asn_DEF_MO_forward_specs /* Additional specs */
};
