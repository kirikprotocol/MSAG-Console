CREATE TABLE SMS_STAT_SMS_--PERIOD-- 
(
  period       NUMBER(22)     NOT NULL,
  accepted     NUMBER(22)     NOT NULL,
  rejected     NUMBER(22)     NOT NULL,
  delivered    NUMBER(22)     NOT NULL,
  failed       NUMBER(22)     NOT NULL,
  rescheduled  NUMBER(22)     NOT NULL,
  temporal     NUMBER(22)     NOT NULL,
  peak_i       NUMBER(22)     DEFAULT 0 NOT NULL,
  peak_o       NUMBER(22)     DEFAULT 0 NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX S1_IDX_--PERIOD-- ON SMS_STAT_SMS_--PERIOD-- (period)
TABLESPACE SMSC_IDX;

CREATE TABLE SMS_STAT_STATE_--PERIOD--
(
  period       NUMBER(22)     NOT NULL,
  errcode      NUMBER(22)     NOT NULL,
  counter      NUMBER(22)     NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX S2_IDX_--PERIOD-- ON SMS_STAT_STATE_--PERIOD-- (period)
TABLESPACE SMSC_IDX;
CREATE INDEX S3_IDX_--PERIOD-- ON SMS_STAT_STATE_--PERIOD-- (errcode)
TABLESPACE SMSC_IDX;

CREATE TABLE SMS_STAT_SME_--PERIOD--
(
  period       NUMBER(22)     NOT NULL,
  systemid     VARCHAR2(16)   NOT NULL,
  accepted     NUMBER(22)     NOT NULL,
  rejected     NUMBER(22)     NOT NULL,
  delivered    NUMBER(22)     NOT NULL,
  failed       NUMBER(22)     NOT NULL,
  rescheduled  NUMBER(22)     NOT NULL,
  temporal     NUMBER(22)     NOT NULL,	
  peak_i       NUMBER(22)     DEFAULT 0 NOT NULL,
  peak_o       NUMBER(22)     DEFAULT 0 NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX S4_IDX_--PERIOD-- ON SMS_STAT_SME_--PERIOD-- (period)
TABLESPACE SMSC_IDX;
CREATE INDEX S5_IDX_--PERIOD-- ON SMS_STAT_SME_--PERIOD-- (systemid)
TABLESPACE SMSC_IDX;

CREATE TABLE SMS_STAT_SME_STATE_--PERIOD--
(
  period       NUMBER(22)     NOT NULL,
  systemid     VARCHAR2(16)   NOT NULL,
  errcode      NUMBER(22)     NOT NULL,
  counter      NUMBER(22)     NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX S6_IDX_--PERIOD-- ON SMS_STAT_SME_STATE_--PERIOD-- (period)
TABLESPACE SMSC_IDX;
CREATE INDEX S7_IDX_--PERIOD-- ON SMS_STAT_SME_STATE_--PERIOD-- (systemid)
TABLESPACE SMSC_IDX;
CREATE INDEX S8_IDX_--PERIOD-- ON SMS_STAT_SME_STATE_--PERIOD-- (errcode)
TABLESPACE SMSC_IDX;

CREATE TABLE SMS_STAT_ROUTE_--PERIOD--
(
  period       NUMBER(22)     NOT NULL,
  routeid      VARCHAR2(32)   NOT NULL,
  accepted     NUMBER(22)     NOT NULL,
  rejected     NUMBER(22)     NOT NULL,
  delivered    NUMBER(22)     NOT NULL,
  failed       NUMBER(22)     NOT NULL,
  rescheduled  NUMBER(22)     NOT NULL,
  temporal     NUMBER(22)     NOT NULL,
  peak_i       NUMBER(22)     DEFAULT 0 NOT NULL,
  peak_o       NUMBER(22)     DEFAULT 0 NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX S9_IDX_--PERIOD-- ON SMS_STAT_ROUTE_--PERIOD-- (period)
TABLESPACE SMSC_IDX;
CREATE INDEX S10_IDX_--PERIOD-- ON SMS_STAT_ROUTE_--PERIOD-- (routeid)
TABLESPACE SMSC_IDX;

CREATE TABLE SMS_STAT_ROUTE_STATE_--PERIOD--
(
  period       NUMBER(22)     NOT NULL,
  routeid      VARCHAR2(32)   NOT NULL,
  errcode      NUMBER(22)     NOT NULL,
  counter      NUMBER(22)     NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX S11_IDX_--PERIOD-- ON SMS_STAT_ROUTE_STATE_--PERIOD-- (period)
TABLESPACE SMSC_IDX;
CREATE INDEX S12_IDX_--PERIOD-- ON SMS_STAT_ROUTE_STATE_--PERIOD-- (routeid)
TABLESPACE SMSC_IDX;
CREATE INDEX S13_IDX_--PERIOD-- ON SMS_STAT_ROUTE_STATE_--PERIOD-- (errcode)
TABLESPACE SMSC_IDX;

quit;