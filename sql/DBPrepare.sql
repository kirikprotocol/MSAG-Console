
-- ********************* Operative tables for SMSC ********************* --

DROP TABLE SMS_MSG;
CREATE TABLE SMS_MSG
(  
   ID             NUMBER(22)   NOT NULL 
                  CONSTRAINT SMS_MSG_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX INITRANS 40,
   ST             NUMBER(3)    NOT NULL,
   SUBMIT_TIME    DATE         NOT NULL,   
   VALID_TIME     DATE         NOT NULL,
   ATTEMPTS       NUMBER(22)   NOT NULL,
   LAST_RESULT    NUMBER(22)   NOT NULL,
   LAST_TRY_TIME  DATE         NULL,
   NEXT_TRY_TIME  DATE         NULL, 
   OA             VARCHAR2(30) NOT NULL,   
   DA             VARCHAR2(30) NOT NULL,   
   DDA            VARCHAR2(30) NOT NULL,   
   MR             NUMBER(5)    NOT NULL,
   SVC_TYPE       VARCHAR2(6)  NULL,
   ARC            CHAR(1)      NOT NULL,
   DR             NUMBER(3)    NOT NULL,
   BR             NUMBER(3)    NOT NULL,
   SRC_MSC        VARCHAR2(21) NULL,
   SRC_IMSI       VARCHAR2(21) NULL,
   SRC_SME_N      NUMBER(22)   NOT NULL,   
   DST_MSC        VARCHAR2(21) NULL,
   DST_IMSI       VARCHAR2(21) NULL,
   DST_SME_N      NUMBER(22)   NULL,
   ROUTE_ID       VARCHAR2(32) NULL,
   SVC_ID         NUMBER(22)   NULL,
   PRTY           NUMBER(22)   NULL,
   SRC_SME_ID     VARCHAR2(15) NULL,
   DST_SME_ID     VARCHAR2(15) NULL,
   TXT_LENGTH     NUMBER(10)   NULL,
   MSG_REF        NUMBER(5)    NULL,
   SEQ_NUM        NUMBER(3)    NOT NULL,
   BODY_LEN       NUMBER(10)   NOT NULL,
   BODY           RAW(1500)    NULL
) TABLESPACE SMSC_DATA INITRANS 40;

DROP SEQUENCE SMS_MSG_SEQ;
CREATE SEQUENCE SMS_MSG_SEQ INCREMENT BY 1000
START WITH 0 MINVALUE 0 MAXVALUE 10000000000000000000000 CACHE 100;

CREATE INDEX SMS_MSG_OA_IDX ON SMS_MSG (OA)
TABLESPACE SMSC_IDX INITRANS 40;

CREATE INDEX SMS_MSG_DA_IDX ON SMS_MSG (DA)
TABLESPACE SMSC_IDX INITRANS 40;

CREATE INDEX SMS_MSG_DDA_IDX ON SMS_MSG (DDA)
TABLESPACE SMSC_IDX INITRANS 40;

CREATE INDEX SMS_MSG_NEXT_TIME_IDX ON SMS_MSG (NEXT_TRY_TIME)
TABLESPACE SMSC_IDX INITRANS 40;

CREATE INDEX SMS_MSG_SUBMIT_TIME_IDX ON SMS_MSG (SUBMIT_TIME)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

CREATE INDEX SMS_MSG_SRC_SME_ID_IDX ON SMS_MSG (SRC_SME_ID)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

CREATE INDEX SMS_MSG_DST_SME_ID_IDX ON SMS_MSG (DST_SME_ID)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

CREATE INDEX SMS_MSG_ROUTE_ID_IDX ON SMS_MSG (ROUTE_ID)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

DROP TABLE SMS_ATCH;
CREATE TABLE SMS_ATCH
(
   ID       NUMBER(22)   NOT NULL
            CONSTRAINT SMS_ATCH_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX  INITRANS 40,
   BODY     BLOB     NOT NULL
) TABLESPACE SMSC_DATA  INITRANS 40;

DROP TABLE SMS_PROFILE;
CREATE TABLE SMS_PROFILE (
  MASK        VARCHAR2(30),
  REPORTINFO  CHAR(3),
  CODESET     CHAR(3),
  LOCALE      VARCHAR2(32),
  HIDDEN      CHAR,
  HIDDEN_MOD  CHAR,
  DIVERT      VARCHAR2(30),
  DIVERT_ACT  CHAR(5),
  DIVERT_MOD  CHAR,
  UDHCONCAT   CHAR
) TABLESPACE SMSC_DATA INITRANS 40;

CREATE UNIQUE INDEX SMS_PROFILE_MASK_IDX ON SMS_PROFILE (MASK)
TABLESPACE SMSC_IDX INITRANS 40;

-- ********************** Archive table for SMSC ********************** --

DROP TABLE SMS_ARC;
CREATE TABLE SMS_ARC
(  
   ID             NUMBER(22)   NOT NULL 
                  CONSTRAINT SMS_ARC_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX INITRANS 40,
   ST             NUMBER(3)    NOT NULL,
   SUBMIT_TIME    DATE         NOT NULL,   
   VALID_TIME     DATE         NOT NULL,
   ATTEMPTS       NUMBER(22)   NOT NULL,
   LAST_RESULT    NUMBER(22)   NOT NULL,
   LAST_TRY_TIME  DATE         NULL,
   NEXT_TRY_TIME  DATE         NULL, 
   OA             VARCHAR2(30) NOT NULL,   
   DA             VARCHAR2(30) NOT NULL,   
   DDA            VARCHAR2(30) NOT NULL,   
   MR             NUMBER(5)    NOT NULL,
   SVC_TYPE       VARCHAR2(6)  NULL,
   DR             NUMBER(3)    NOT NULL,
   BR             NUMBER(3)    NOT NULL,
   SRC_MSC        VARCHAR2(21) NULL,
   SRC_IMSI       VARCHAR2(21) NULL,
   SRC_SME_N      NUMBER(22)   NOT NULL,   
   DST_MSC        VARCHAR2(21) NULL,
   DST_IMSI       VARCHAR2(21) NULL,
   DST_SME_N      NUMBER(22)   NULL,
   ROUTE_ID       VARCHAR2(32) NULL,
   SVC_ID         NUMBER(22)   NULL,
   PRTY           NUMBER(22)   NULL, 
   SRC_SME_ID     VARCHAR2(15) NULL,
   DST_SME_ID     VARCHAR2(15) NULL,
   TXT_LENGTH     NUMBER(10)   NULL,
   BODY_LEN       NUMBER(10)   NOT NULL,
   BODY           RAW(1500)    NULL
) TABLESPACE SMSC_DATA  INITRANS 40;

CREATE INDEX SMS_ARC_LAST_TRY_TIME_IDX ON SMS_ARC (LAST_TRY_TIME)
TABLESPACE SMSC_IDX  INITRANS 40;

CREATE INDEX SMS_ARC_OA_IDX ON SMS_ARC (OA)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

CREATE INDEX SMS_ARC_DA_IDX ON SMS_ARC (DDA)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

CREATE INDEX SMS_ARC_SUBMIT_TIME_IDX ON SMS_ARC (SUBMIT_TIME)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

CREATE INDEX SMS_ARC_SRC_SME_ID_IDX ON SMS_ARC (SRC_SME_ID)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

CREATE INDEX SMS_ARC_DST_SME_ID_IDX ON SMS_ARC (DST_SME_ID)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

CREATE INDEX SMS_ARC_ROUTE_ID_IDX ON SMS_ARC (ROUTE_ID)
TABLESPACE SMSC_IDX NOLOGGING INITRANS 40;

CREATE OR REPLACE PROCEDURE DO_FINALIZE_SMS 
  (A_id IN NUMBER, A_st IN NUMBER, A_submitTime IN DATE, A_validTime IN DATE,
   A_attempts IN NUMBER, A_lastResult IN NUMBER, 
   A_lastTryTime IN DATE, A_nextTryTime IN DATE,
   A_oa IN VARCHAR2, A_oaVal IN VARCHAR2, A_oaTon IN NUMBER, A_oaNpi IN NUMBER,
   A_da IN VARCHAR2, A_daVal IN VARCHAR2, A_daTon IN NUMBER, A_daNpi IN NUMBER,
   A_dda IN VARCHAR2,
   A_mr IN NUMBER, A_svcType IN VARCHAR2, A_dr IN NUMBER, A_br IN NUMBER,
   A_srcMsc IN VARCHAR2, A_srcImsi IN VARCHAR2, A_srcSmeN IN NUMBER,
   A_dstMsc IN VARCHAR2, A_dstImsi IN VARCHAR2, A_dstSmeN IN NUMBER,
   A_routeId IN VARCHAR2, A_svcId IN NUMBER, A_prty IN NUMBER, 
   A_srcSmeId IN VARCHAR2, A_dstSmeId VARCHAR2, A_txtLength IN NUMBER,
   A_bodyLen IN NUMBER, A_body IN RAW, A_arc IN NUMBER, A_bill IN NUMBER, 
   A_delete IN NUMBER, A_diverted IN NUMBER)
IS
BEGIN
   
   IF A_arc != 0 THEN
    INSERT INTO SMS_ARC
	(ID, ST, SUBMIT_TIME, VALID_TIME, ATTEMPTS, LAST_RESULT,
	 LAST_TRY_TIME, NEXT_TRY_TIME, OA, DA, DDA, MR, SVC_TYPE, DR, BR, 
	 SRC_MSC, SRC_IMSI, SRC_SME_N, DST_MSC, DST_IMSI, DST_SME_N,
	 ROUTE_ID, SVC_ID, PRTY, SRC_SME_ID, DST_SME_ID, 
	 TXT_LENGTH, BODY_LEN, BODY) 
    VALUES
	(A_id, A_st, A_submitTime, A_validTime, A_attempts, A_lastResult,
	 A_lastTryTime, A_nextTryTime, A_oa, A_da, A_dda, A_mr, A_svcType, A_dr, A_br,
	 A_srcMsc, A_srcImsi, A_srcSmeN, A_dstMsc, A_dstImsi, A_dstSmeN,
	 A_routeId, A_svcId, A_prty, A_srcSmeId, A_dstSmeId, 
	 A_txtLength, A_bodyLen, A_body);
   END IF;
   
   IF A_delete != 0 THEN
       DELETE FROM SMS_MSG WHERE ID=A_id;
   END IF;

END DO_FINALIZE_SMS;
/
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
  temporal     NUMBER(22)     NOT NULL,	
  peak_i       NUMBER(22)     DEFAULT 0 NOT NULL,
  peak_o       NUMBER(22)     DEFAULT 0 NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_STAT_SME_IDX ON SMS_STAT_SME (period)
TABLESPACE SMSC_IDX;
CREATE INDEX SMS_STAT_SME_ID_IDX ON SMS_STAT_SME (systemid)
TABLESPACE SMSC_IDX;

DROP TABLE SMS_STAT_SME_STATE;
CREATE TABLE SMS_STAT_SME_STATE
(
  period       NUMBER(22)     NOT NULL,
  systemid     VARCHAR2(16)   NOT NULL,
  errcode      NUMBER(22)     NOT NULL,
  counter      NUMBER(22)     NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_STAT_SME_STATE_IDX ON SMS_STAT_SME_STATE (period)
TABLESPACE SMSC_IDX;
CREATE INDEX SMS_STAT_SME_STATE_ID_IDX ON SMS_STAT_SME_STATE (systemid)
TABLESPACE SMSC_IDX;
CREATE INDEX SMS_STAT_SME_STATE_ERR_IDX ON SMS_STAT_SME_STATE (errcode)
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
  temporal     NUMBER(22)     NOT NULL,
  peak_i       NUMBER(22)     DEFAULT 0 NOT NULL,
  peak_o       NUMBER(22)     DEFAULT 0 NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_STAT_ROUTE_IDX ON SMS_STAT_ROUTE (period)
TABLESPACE SMSC_IDX;
CREATE INDEX SMS_STAT_ROUTE_ID_IDX ON SMS_STAT_ROUTE (routeid)
TABLESPACE SMSC_IDX;

DROP TABLE SMS_STAT_ROUTE_STATE;
CREATE TABLE SMS_STAT_ROUTE_STATE
(
  period       NUMBER(22)     NOT NULL,
  routeid      VARCHAR2(32)   NOT NULL,
  errcode      NUMBER(22)     NOT NULL,
  counter      NUMBER(22)     NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_STAT_ROUTE_STATE_IDX ON SMS_STAT_ROUTE_STATE (period)
TABLESPACE SMSC_IDX;
CREATE INDEX SMS_STAT_ROUTE_STATE_ID_IDX ON SMS_STAT_ROUTE_STATE (routeid)
TABLESPACE SMSC_IDX;
CREATE INDEX SMS_STAT_ROUTE_STATE_ERR_IDX ON SMS_STAT_ROUTE_STATE (errcode)
TABLESPACE SMSC_IDX;

-- ********************* Commutator lock table ********************* --

DROP TABLE MSC_LOCK;
CREATE TABLE MSC_LOCK 
(
  MSC          VARCHAR2(21)   NOT NULL
  	       CONSTRAINT MSC_LOCK_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX,
  M_LOCK       CHAR(1)        NULL,
  A_LOCK       CHAR(1)        NULL,
  F_COUNT      NUMBER(22)     NOT NULL
) TABLESPACE SMSC_DATA;

-- ********************* ACL tables ******************************** --

DROP TABLE SMS_ACLINFO;
CREATE TABLE SMS_ACLINFO 
(
  ID           INTEGER NOT NULL
  	       CONSTRAINT SMS_ACLINFO_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX,
  NAME         VARCHAR2(32) NOT NULL
  	       CONSTRAINT SMS_ACLINFO_NAME UNIQUE USING INDEX TABLESPACE SMSC_IDX,
  DESCRIPTION  VARCHAR2(128),
  CACHE_TYPE   CHAR(1)
) TABLESPACE SMSC_DATA;


DROP TABLE SMS_ACL;
CREATE TABLE SMS_ACL 
(
  ID           INTEGER NOT NULL,
  ADDRESS      VARCHAR2(30) NOT NULL,
  PRIMARY KEY  (ID,ADDRESS),
  FOREIGN KEY  (ID) REFERENCES SMS_ACLINFO (ID) ON DELETE CASCADE
) ORGANIZATION INDEX TABLESPACE SMSC_DATA;

DROP SEQUENCE SMS_ACLINFO_SEQ;
CREATE SEQUENCE SMS_ACLINFO_SEQ INCREMENT BY 1000
START WITH 1 MINVALUE 1 MAXVALUE 1000000 CACHE 100;
