
-- ##### Patch for DB created by DBPrepare.sql (v1.44 -> v1.45/v1.46) #####

ALTER TABLE SMS_PROFILE ADD HIDDEN NUMBER(3);
UPDATE SMS_PROFILE SET HIDDEN=1;