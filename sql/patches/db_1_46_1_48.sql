
-- ##### Patch for DB created by DBPrepare.sql (v1.46 -> v1.47/1.48) #####

CREATE INDEX SMS_ARC_LAST_TRY_TIME_IDX ON SMS_ARC (LAST_TRY_TIME)
TABLESPACE SMSC_IDX;
