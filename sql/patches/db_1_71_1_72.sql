
-- ****************** Statistics tables for SMSC updates ****************** --

ALTER TABLE SMS_STAT_SMS MODIFY (
  peak_i NUMBER(22) DEFAULT 0, 
  peak_o NUMBER(22) DEFAULT 0
);

ALTER TABLE SMS_STAT_SME MODIFY (
  peak_i NUMBER(22) DEFAULT 0, 
  peak_o NUMBER(22) DEFAULT 0
);

ALTER TABLE SMS_STAT_ROUTE MODIFY (
  peak_i NUMBER(22) DEFAULT 0,
  peak_o NUMBER(22) DEFAULT 0
);
