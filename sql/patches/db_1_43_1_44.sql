
-- ##### Patch for DB created by DBPrepare.sql (v1.43 -> v1.44) #####

ALTER TABLE SMS_PROFILE ADD LOCALE VARCHAR2(32);
UPDATE SMS_PROFILE SET LOCALE='en_en';