
-- ********************* Operative tables for SMSC ********************* --

DROP TABLE SMS_MSG;
CREATE TABLE SMS_MSG
(  
   ID             NUMBER(22)   NOT NULL 
                  CONSTRAINT SMS_MSG_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX NOSORT,
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
   MSG_REF        NUMBER(3)    NULL,
   SEQ_NUM        NUMBER(3)    NOT NULL,
   BODY_LEN       NUMBER(10)   NOT NULL,
   BODY           RAW(1500)    NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_MSG_OA_IDX ON SMS_MSG (OA)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_MSG_DA_IDX ON SMS_MSG (DA)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_MSG_DDA_IDX ON SMS_MSG (DDA)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_MSG_NEXT_TIME_IDX ON SMS_MSG (NEXT_TRY_TIME)
TABLESPACE SMSC_IDX;

CREATE BITMAP INDEX SMS_MSG_MSG_REF_IDX ON SMS_MSG (MSG_REF)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_MSG_SUBMIT_TIME_IDX ON SMS_MSG (SUBMIT_TIME)
TABLESPACE SMSC_IDX NOSORT NOLOGGING;

CREATE BITMAP INDEX SMS_MSG_SRC_SME_ID_IDX ON SMS_MSG (SRC_SME_ID)
TABLESPACE SMSC_IDX NOLOGGING;

CREATE BITMAP INDEX SMS_MSG_DST_SME_ID_IDX ON SMS_MSG (DST_SME_ID)
TABLESPACE SMSC_IDX NOLOGGING;

CREATE BITMAP INDEX SMS_MSG_ROUTE_ID_IDX ON SMS_MSG (ROUTE_ID)
TABLESPACE SMSC_IDX NOLOGGING;

DROP TABLE SMS_ATCH;
CREATE TABLE SMS_ATCH
(
   ID       NUMBER(22)   NOT NULL
            CONSTRAINT SMS_ATCH_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX NOSORT,
   BODY     BLOB     NOT NULL
) TABLESPACE SMSC_DATA;

DROP TABLE SMS_IDS;
CREATE TABLE SMS_IDS
(  
   ID        NUMBER(22) NOT NULL 
             CONSTRAINT SMS_IDS_PK PRIMARY KEY
)
ORGANIZATION INDEX TABLESPACE SMSC_IDX;

DROP TABLE SMS_PROFILE;
CREATE TABLE SMS_PROFILE (
  MASK      VARCHAR2(30),
  REPORTINFO   NUMBER(3),
  CODESET   NUMBER(3),
  LOCALE    VARCHAR2(32),
  HIDDEN   NUMBER(3),
  HIDDEN_MOD CHAR
);

CREATE UNIQUE INDEX SMS_PROFILE_MASK_IDX ON SMS_PROFILE (MASK)
TABLESPACE SMSC_IDX;

-- ********************** Archive table for SMSC ********************** --

DROP TABLE SMS_ARC;
CREATE TABLE SMS_ARC
(  
   ID             NUMBER(22)   NOT NULL 
                  CONSTRAINT SMS_ARC_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX,
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
) TABLESPACE SMSC_DATA;

CREATE OR REPLACE PROCEDURE DO_FINALIZE_SMS 
  (id IN NUMBER, st IN NUMBER, submitTime IN DATE, validTime IN DATE,
   attempts IN NUMBER, lastResult IN NUMBER, 
   lastTryTime IN DATE, nextTryTime IN DATE,
   oa IN VARCHAR2, oaVal IN VARCHAR2, oaTon IN NUMBER, oaNpi IN NUMBER,
   da IN VARCHAR2, daVal IN VARCHAR2, daTon IN NUMBER, daNpi IN NUMBER,
   dda IN VARCHAR2,
   mr IN NUMBER, svcType IN VARCHAR2, dr IN NUMBER, br IN NUMBER,
   srcMsc IN VARCHAR2, srcImsi IN VARCHAR2, srcSmeN IN NUMBER,
   dstMsc IN VARCHAR2, dstImsi IN VARCHAR2, dstSmeN IN NUMBER,
   routeId IN VARCHAR2, svcId IN NUMBER, prty IN NUMBER, 
   srcSmeId IN VARCHAR2, dstSmeId VARCHAR2, txtLength IN NUMBER,
   bodyLen IN NUMBER, body IN RAW, arc IN NUMBER, bill IN NUMBER)
IS
BEGIN
   
   DELETE FROM SMS_MSG WHERE ID=id;
   
   IF arc != 0 THEN
    INSERT INTO SMS_ARC
	(ID, ST, SUBMIT_TIME, VALID_TIME, ATTEMPTS, LAST_RESULT,
	 LAST_TRY_TIME, NEXT_TRY_TIME, OA, DA, DDA, MR, SVC_TYPE, DR, BR, 
	 SRC_MSC, SRC_IMSI, SRC_SME_N, DST_MSC, DST_IMSI, DST_SME_N,
	 ROUTE_ID, SVC_ID, PRTY, SRC_SME_ID, DST_SME_ID, 
	 TXT_LENGTH, BODY_LEN, BODY) 
    VALUES
	(id, st, submitTime, validTime, attempts, lastResult,
	 lastTryTime, nextTryTime, oa, da, dda, mr, svcType, dr, br,
	 srcMsc, srcImsi, srcSmeN, dstMsc, dstImsi, dstSmeN,
	 routeId, svcId, prty, srcSmeId, dstSmeId, 
	 txtLength, bodyLen, body);
   END IF;
   
   IF bill != 0 THEN
	CREATE_BILLING_RECORD(id, 
	    oaVal, oaTon, oaNpi, srcMsc, srcImsi, 
	    daVal, daTon, daNpi, dstMsc, dstImsi,
	    submitTime, lastTryTime, st, routeId, svcId, txtLength);
   END IF;    	    

END DO_FINALIZE_SMS;
/

CREATE INDEX SMS_ARC_LAST_TRY_TIME_IDX ON SMS_ARC (LAST_TRY_TIME)
TABLESPACE SMSC_IDX NOSORT;

CREATE INDEX SMS_ARC_OA_IDX ON SMS_ARC (OA)
TABLESPACE SMSC_IDX NOLOGGING;

CREATE INDEX SMS_ARC_DA_IDX ON SMS_ARC (DA)
TABLESPACE SMSC_IDX NOLOGGING;

CREATE INDEX SMS_ARC_DDA_IDX ON SMS_ARC (DDA)
TABLESPACE SMSC_IDX NOLOGGING;

CREATE INDEX SMS_ARC_SUBMIT_TIME_IDX ON SMS_ARC (SUBMIT_TIME)
TABLESPACE SMSC_IDX NOLOGGING;

CREATE BITMAP INDEX SMS_ARC_SRC_SME_ID_IDX ON SMS_ARC (SRC_SME_ID)
TABLESPACE SMSC_IDX NOLOGGING;

CREATE BITMAP INDEX SMS_ARC_DST_SME_ID_IDX ON SMS_ARC (DST_SME_ID)
TABLESPACE SMSC_IDX NOLOGGING;

CREATE BITMAP INDEX SMS_ARC_ROUTE_ID_IDX ON SMS_ARC (ROUTE_ID)
TABLESPACE SMSC_IDX NOLOGGING;


-- ********************** Billing table for SMSC ********************** --

DROP TABLE SMS_BILL;
CREATE TABLE SMS_BILL
(
   ID             NUMBER         NOT NULL    -- generated via sequence
   		  CONSTRAINT SMS_BILL_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX,
   MSG_ID         NUMBER(22)     NOT NULL,   -- ID in SMS_MSG
   CALL_DIRECTION VARCHAR2(5)    NOT NULL,   -- filled by stored procedure
   RECORD_TYPE    NUMBER         NOT NULL,   -- filled by stored procedure
   SUBMIT         DATE           NOT NULL,   -- SUBMIT_TIME from SMS_MSG
   FINALIZED      DATE           NULL,       -- LAST_TIME from SMS_MSG
   STATUS         NUMBER         NOT NULL,   -- LAST_RESULT from SMS_MSG
   PAYER_ADDR     VARCHAR2(21)   NOT NULL,   -- filled by stored procedure (by OA/DDA)
   PAYER_TON      NUMBER         NOT NULL,   -- filled by stored procedure (by OA/DDA)
   PAYER_NPI      NUMBER         NOT NULL,   -- filled by stored procedure (by OA/DDA)
   PAYER_IMSI     VARCHAR2(21)   NULL,       -- filled by stored procedure (by SRC/DST IMSI)
   PAYER_MSC      VARCHAR2(21)   NULL,       -- filled by stored procedure (by SRC/DST MSC)
   OTHER_ADDR     VARCHAR2(21)   NULL,       -- filled by stored procedure (by OA/DDA)
   OTHER_TON      NUMBER         NULL,       -- filled by stored procedure (by OA/DDA)
   OTHER_NPI      NUMBER         NULL,       -- filled by stored procedure (by OA/DDA)
   ROUTE_ID       VARCHAR2(32)   NULL,       -- ROUTE_ID from SMS_MSG
   SERVICE_CODE   NUMBER         NULL,       -- SERVICE_ID from SMS_MSG
   TXT_LENGTH     NUMBER         NULL,       -- add it to SMS_MSG
   UPLOAD_ID      NUMBER         NULL        -- skip it (used externally)
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_BILL_UPLOAD_ID_IDX ON SMS_BILL (UPLOAD_ID) 
TABLESPACE SMSC_IDX;

DROP SEQUENCE SMS_BILL_SEQ;
CREATE SEQUENCE SMS_BILL_SEQ INCREMENT BY 1 START WITH 1 CACHE 20;

CREATE OR REPLACE PROCEDURE CREATE_BILLING_RECORD 
  (smsId IN NUMBER, 
   smsOaVal IN VARCHAR2, smsOaTon IN NUMBER, smsOaNpi IN NUMBER,
   smsOaMsc IN VARCHAR2, smsOaImsi IN VARCHAR2,
   smsDaVal IN VARCHAR2, smsDaTon IN NUMBER, smsDaNpi IN NUMBER,
   smsDaMsc IN VARCHAR2, smsDaImsi IN VARCHAR2,
   smsSubmit IN DATE, smsFinalized IN DATE, smsStatus IN NUMBER,
   smsRouteId IN VARCHAR2, smsServiceId IN NUMBER, smsTxtLen IN NUMBER) 
IS
   billId            NUMBER;
BEGIN
   
   SELECT SMS_BILL_SEQ.NEXTVAL INTO billId FROM DUAL;
   INSERT INTO SMS_BILL 
     (ID, MSG_ID, CALL_DIRECTION, RECORD_TYPE, SUBMIT, FINALIZED, STATUS,
      PAYER_ADDR, PAYER_TON, PAYER_NPI, PAYER_IMSI, PAYER_MSC,
      OTHER_ADDR, OTHER_TON, OTHER_NPI,
      ROUTE_ID, SERVICE_CODE, TXT_LENGTH) 
   VALUES 
     (billId, smsId, 'O', 10, smsSubmit, smsFinalized, smsStatus, 
      smsOaVal, smsOaTon, smsOaNpi, smsOaImsi, smsOaMsc,
      smsDaVal, smsDaTon, smsDaNpi, 
      smsRouteId, smsServiceId, smsTxtLen);
   
   SELECT SMS_BILL_SEQ.NEXTVAL INTO billId FROM DUAL;
   INSERT INTO SMS_BILL 
     (ID, MSG_ID, CALL_DIRECTION, RECORD_TYPE, SUBMIT, FINALIZED, STATUS,
      PAYER_ADDR, PAYER_TON, PAYER_NPI, PAYER_IMSI, PAYER_MSC,
      OTHER_ADDR, OTHER_TON, OTHER_NPI, 
      ROUTE_ID, SERVICE_CODE, TXT_LENGTH) 
   VALUES 
     (billId, smsId, 'I', 20, smsSubmit, smsFinalized, smsStatus, 
      smsDaVal, smsDaTon, smsDaNpi, smsDaImsi, smsDaMsc,
      smsOaVal, smsOaTon, smsOaNpi, 
      smsRouteId, smsServiceId, smsTxtLen);

END CREATE_BILLING_RECORD;
/

-- ********************* Statistics tables for SMSC ********************* --

DROP TABLE SMS_STAT_SMS;
CREATE TABLE SMS_STAT_SMS 
(
  period       NUMBER(22)     NOT NULL,
  received     NUMBER(22)     NOT NULL,
  finalized    NUMBER(22)     NOT NULL,
  rescheduled  NUMBER(22)     NOT NULL
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
  received     NUMBER(22)     NOT NULL,
  sent         NUMBER(22)     NOT NULL
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
  processed    NUMBER(22)     NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX SMS_STAT_ROUTE_IDX ON SMS_STAT_ROUTE (period)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_STAT_ROUTE_ID_IDX ON SMS_STAT_ROUTE (routeid)
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
