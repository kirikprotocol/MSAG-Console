
-- ********************* Operative tables for SMSC ********************* --

DROP TABLE SMS_MSG;
CREATE TABLE SMS_MSG
(  
   ID             RAW(8)       NOT NULL,
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
   ROUTE_ID       VARCHAR2(20) NULL,
   SVC_ID         NUMBER(22)   NULL,
   PRTY           NUMBER(22)   NULL,
   SRC_SME_ID     VARCHAR2(15) NULL,
   DST_SME_ID     VARCHAR2(15) NULL,
   TXT_LENGTH     NUMBER(10)   NULL,
   BODY_LEN       NUMBER(10)   NOT NULL,
   BODY           RAW(1550)    NULL
) TABLESPACE SMSC_DATA;

CREATE UNIQUE INDEX SMS_MSG_IDX ON SMS_MSG (ID)
TABLESPACE SMSC_IDX NOSORT;

ALTER TABLE SMS_MSG ADD CONSTRAINT SMS_MSG_PK PRIMARY KEY (ID);

CREATE INDEX SMS_MSG_OA_IDX ON SMS_MSG (OA)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_MSG_DA_IDX ON SMS_MSG (DA)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_MSG_NEXT_TIME_IDX ON SMS_MSG (NEXT_TRY_TIME)
TABLESPACE SMSC_IDX;

DROP TABLE SMS_ATCH;
CREATE TABLE SMS_ATCH
(
   ID       RAW(8)   NOT NULL,
   BODY     BLOB     NOT NULL
) TABLESPACE SMSC_IDX;

CREATE UNIQUE INDEX SMS_ATCH_IDX ON SMS_ATCH (ID)
TABLESPACE SMSC_IDX NOSORT;

ALTER TABLE SMS_ATCH ADD CONSTRAINT SMS_ATCH_PK PRIMARY KEY (ID);

DROP TABLE SMS_IDS;
CREATE TABLE SMS_IDS
(  
   ID        RAW(8)      NOT NULL
) TABLESPACE SMSC_DATA;

CREATE UNIQUE INDEX SMS_IDS_IDX ON SMS_IDS (ID)
TABLESPACE SMSC_IDX;

DROP TABLE SMS_PROFILE;
CREATE TABLE SMS_PROFILE (
  MASK      VARCHAR2(30),
  REPORTINFO   NUMBER(3),
  CODESET   NUMBER(3)
);

CREATE UNIQUE INDEX SMS_PROFILE_MASK_IDX ON SMS_PROFILE (MASK)
TABLESPACE SMSC_IDX;

-- ********************** Archive table for SMSC ********************** --

DROP TABLE SMS_ARC;
CREATE TABLE SMS_ARC
(  
   ID             RAW(8)       NOT NULL,
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
   ROUTE_ID       VARCHAR2(20) NULL,
   SVC_ID         NUMBER(22)   NULL,
   PRTY           NUMBER(22)   NULL, 
   SRC_SME_ID     VARCHAR2(15) NULL,
   DST_SME_ID     VARCHAR2(15) NULL,
   TXT_LENGTH     NUMBER(10)   NULL,
   BODY_LEN       NUMBER(10)   NOT NULL,
   BODY           RAW(1550)    NULL
)TABLESPACE SMSC_DATA;

CREATE UNIQUE INDEX SMS_ARC_IDX ON SMS_ARC (ID)
TABLESPACE SMSC_IDX;

ALTER TABLE SMS_ARC ADD CONSTRAINT SMS_ARC_PK PRIMARY KEY (ID);

-- ********************** Billing table for SMSC ********************** --

DROP TABLE SMS_BILL;
CREATE TABLE SMS_BILL
(
   ID             NUMBER         NOT NULL,   -- generated via sequence
   MSG_ID         RAW(8)         NOT NULL,   -- ID in SMS_MSG
   CALL_DIRECTION VARCHAR2(5)    NOT NULL,   -- filled by stored procedure
   RECORD_TYPE    NUMBER         NOT NULL,   -- filled by stored procedure
   SUBMIT         DATE           NOT NULL,   -- SUBMIT_TIME from SMS_MSG
   FINALIZED      DATE           NULL,       -- LAST_TIME from SMS_MSG
   STATUS         NUMBER         NOT NULL,   -- LAST_RESULT from SMS_MSG
   PAYER_ADDR     VARCHAR2(21)   NOT NULL,   -- filled by stored procedure (by OA/DDA)
   PAYER_TON      NUMBER         NOT NULL,   -- filled by stored procedure (by OA/DDA)
   PAYER_NPI      NUMBER         NOT NULL,   -- filled by stored procedure (by OA/DDA)
   PAYER_IMSI     VARCHAR2(21)   NULL,       -- filled by stored procedure (by SRC/DST IMSI)
   OTHER_ADDR     VARCHAR2(21)   NULL,       -- filled by stored procedure (by OA/DDA)
   OTHER_TON      NUMBER         NULL,       -- filled by stored procedure (by OA/DDA)
   OTHER_NPI      NUMBER         NULL,       -- filled by stored procedure (by OA/DDA)
   OTHER_MSC      VARCHAR2(21)   NULL,       -- filled by stored procedure (by SRC/DST MSC)
   ROUTE_ID       VARCHAR2(21)   NULL,       -- ROUTE_ID from SMS_MSG
   SERVICE_CODE   NUMBER         NULL,       -- SERVICE_ID from SMS_MSG
   TXT_LENGTH     NUMBER         NULL,       -- add it to SMS_MSG
   UPLOAD_ID      NUMBER         NULL        -- skip it (used externally)
) TABLESPACE SMSC_DATA;

CREATE UNIQUE INDEX SMS_BILL_IDX ON SMS_BILL (ID)
TABLESPACE SMSC_IDX;

ALTER TABLE SMS_BILL ADD CONSTRAINT SMS_BILL_PK PRIMARY KEY (ID);

DROP SEQUENCE SMS_BILL_SEQ;
CREATE SEQUENCE SMS_BILL_SEQ INCREMENT BY 1 START WITH 1 CACHE 20;

CREATE OR REPLACE PROCEDURE CREATE_BILLING_RECORD 
  (smsId IN RAW, 
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
      PAYER_ADDR, PAYER_TON, PAYER_NPI, PAYER_IMSI,
      OTHER_ADDR, OTHER_TON, OTHER_NPI, OTHER_MSC,
      ROUTE_ID, SERVICE_CODE, TXT_LENGTH) 
   VALUES 
     (billId, smsId, 'O', 10, smsSubmit, smsFinalized, smsStatus, 
      smsOaVal, smsOaTon, smsOaNpi, smsOaImsi,
      smsDaVal, smsDaTon, smsDaNpi, smsDaMsc,
      smsRouteId, smsServiceId, smsTxtLen);
   
   SELECT SMS_BILL_SEQ.NEXTVAL INTO billId FROM DUAL;
   INSERT INTO SMS_BILL 
     (ID, MSG_ID, CALL_DIRECTION, RECORD_TYPE, SUBMIT, FINALIZED, STATUS,
      PAYER_ADDR, PAYER_TON, PAYER_NPI, PAYER_IMSI,
      OTHER_ADDR, OTHER_TON, OTHER_NPI, OTHER_MSC,
      ROUTE_ID, SERVICE_CODE, TXT_LENGTH) 
   VALUES 
     (billId, smsId, 'I', 20, smsSubmit, smsFinalized, smsStatus, 
      smsDaVal, smsDaTon, smsDaNpi, smsDaImsi,
      smsOaVal, smsOaTon, smsOaNpi, smsOaMsc,
      smsRouteId, smsServiceId, smsTxtLen);

END CREATE_BILLING_RECORD;
/

