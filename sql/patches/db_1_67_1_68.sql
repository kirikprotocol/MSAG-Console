
-- ********************* Statistics tables for SMSC ********************* --

DROP TABLE SMS_STAT_SMS;
CREATE TABLE SMS_STAT_SMS 
(
  period       NUMBER(22)     NOT NULL,
  accepted     NUMBER(22)     NOT NULL,
  rejected     NUMBER(22)     NOT NULL,
  delivered    NUMBER(22)     NOT NULL,
  failed       NUMBER(22)     NOT NULL,
  rescheduled  NUMBER(22)     NOT NULL,
  temporal     NUMBER(22)     NOT NULL	
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_STAT_SMS_IDX ON SMS_STAT_SMS (period)
TABLESPACE SMSC_IDX;

DROP TABLE SMS_STAT_STATE;
CREATE TABLE SMS_STAT_STATE
(
  period       NUMBER(22)     NOT NULL,
  errcode      NUMBER(22)     NOT NULL,
  counter      NUMBER(22)     NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_STAT_STATE_IDX ON SMS_STAT_STATE (period)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_STAT_STATE_ERR_IDX ON SMS_STAT_STATE (errcode)
TABLESPACE SMSC_IDX;

DROP TABLE SMS_STAT_SME;
CREATE TABLE SMS_STAT_SME
(
  period       NUMBER(22)     NOT NULL,
  systemid     VARCHAR2(16)   NOT NULL,
  accepted     NUMBER(22)     NOT NULL,
  rejected     NUMBER(22)     NOT NULL,
  delivered    NUMBER(22)     NOT NULL,
  failed       NUMBER(22)     NOT NULL,
  rescheduled  NUMBER(22)     NOT NULL,
  temporal     NUMBER(22)     NOT NULL	
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_STAT_SME_IDX ON SMS_STAT_SME (period)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_STAT_SME_ID_IDX ON SMS_STAT_SME (systemid)
TABLESPACE SMSC_IDX;

DROP TABLE SMS_STAT_ROUTE;
CREATE TABLE SMS_STAT_ROUTE
(
  period       NUMBER(22)     NOT NULL,
  routeid      VARCHAR2(32)   NOT NULL,
  accepted     NUMBER(22)     NOT NULL,
  rejected     NUMBER(22)     NOT NULL,
  delivered    NUMBER(22)     NOT NULL,
  failed       NUMBER(22)     NOT NULL,
  rescheduled  NUMBER(22)     NOT NULL,
  temporal     NUMBER(22)     NOT NULL	
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_STAT_ROUTE_IDX ON SMS_STAT_ROUTE (period)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_STAT_ROUTE_ID_IDX ON SMS_STAT_ROUTE (routeid)
TABLESPACE SMSC_IDX;
