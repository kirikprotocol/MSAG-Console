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
  temporal     NUMBER(22)     NOT NULL,
  peak_i       NUMBER(22)     DEFAULT 0 NOT NULL,
  peak_o       NUMBER(22)     DEFAULT 0 NOT NULL
) TABLESPACE @smsc.data.tablespace@;

CREATE INDEX ST_SMS_IDX ON SMS_STAT_SMS (period) TABLESPACE @smsc.index.tablespace@;

DROP TABLE SMS_STAT_STATE;
CREATE TABLE SMS_STAT_STATE
(
  period       NUMBER(22)     NOT NULL,
  errcode      NUMBER(22)     NOT NULL,
  counter      NUMBER(22)     NOT NULL
) TABLESPACE @smsc.data.tablespace@;

CREATE INDEX ST_SMS_ST_IDX ON SMS_STAT_STATE (period,errcode) TABLESPACE @smsc.index.tablespace@;


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
  temporal     NUMBER(22)     NOT NULL,	
  peak_i       NUMBER(22)     DEFAULT 0 NOT NULL,
  peak_o       NUMBER(22)     DEFAULT 0 NOT NULL
) TABLESPACE @smsc.data.tablespace@;

CREATE INDEX ST_SME_IDX ON SMS_STAT_SME (period,systemid) TABLESPACE @smsc.index.tablespace@;

DROP TABLE SMS_STAT_SME_STATE;
CREATE TABLE SMS_STAT_SME_STATE
(
  period       NUMBER(22)     NOT NULL,
  systemid     VARCHAR2(16)   NOT NULL,
  errcode      NUMBER(22)     NOT NULL,
  counter      NUMBER(22)     NOT NULL
) TABLESPACE @smsc.data.tablespace@;

CREATE INDEX ST_SME_ST_IDX ON SMS_STAT_SME_STATE (period,systemid,errcode) TABLESPACE @smsc.index.tablespace@;

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
  temporal     NUMBER(22)     NOT NULL,
  peak_i       NUMBER(22)     DEFAULT 0 NOT NULL,
  peak_o       NUMBER(22)     DEFAULT 0 NOT NULL
) TABLESPACE @smsc.data.tablespace@;

CREATE INDEX ST_ROUTE_IDX ON SMS_STAT_ROUTE (period,routeid) TABLESPACE @smsc.index.tablespace@;

DROP TABLE SMS_STAT_ROUTE_STATE;
CREATE TABLE SMS_STAT_ROUTE_STATE
(
  period       NUMBER(22)     NOT NULL,
  routeid      VARCHAR2(32)   NOT NULL,
  errcode      NUMBER(22)     NOT NULL,
  counter      NUMBER(22)     NOT NULL
) TABLESPACE @smsc.data.tablespace@;

CREATE INDEX ST_ROUTE_ST_IDX ON SMS_STAT_ROUTE_STATE (period,routeid,errcode) TABLESPACE @smsc.index.tablespace@;
