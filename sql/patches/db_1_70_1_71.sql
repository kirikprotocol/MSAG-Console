
-- ****************** Statistics tables for SMSC updates ****************** --

ALTER TABLE SMS_STAT_SMS ADD (
  peak_i NUMBER(22), peak_o NUMBER(22)
);
UPDATE SMS_STAT_SMS SET peak_i=0, peak_o=0;
ALTER TABLE SMS_STAT_SMS MODIFY (
  peak_i NUMBER(22) NOT NULL, peak_o NUMBER(22) NOT NULL
);

ALTER TABLE SMS_STAT_SME ADD (
  peak_i NUMBER(22),
  peak_o NUMBER(22)
);
UPDATE SMS_STAT_SME SET peak_i=0, peak_o=0;
ALTER TABLE SMS_STAT_SME MODIFY (
  peak_i NUMBER(22) NOT NULL, peak_o NUMBER(22) NOT NULL
);

ALTER TABLE SMS_STAT_ROUTE ADD (
  peak_i NUMBER(22),
  peak_o NUMBER(22)
);
UPDATE SMS_STAT_ROUTE SET peak_i=0, peak_o=0;
ALTER TABLE SMS_STAT_ROUTE MODIFY (
  peak_i NUMBER(22) NOT NULL, peak_o NUMBER(22) NOT NULL
);
