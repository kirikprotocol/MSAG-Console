DROP TABLE SMS_MSG_BAK;
CREATE TABLE SMS_MSG_BAK
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

insert into sms_msg_bak
select 
    ID,ST,SUBMIT_TIME,VALID_TIME,ATTEMPTS,
    LAST_RESULT,LAST_TRY_TIME,NEXT_TRY_TIME,
    OA,DA,DDA,MR,SVC_TYPE,ARC,DR,BR,
    SRC_MSC,SRC_IMSI,SRC_SME_N,
    DST_MSC,DST_IMSI,DST_SME_N,
    ROUTE_ID,SVC_ID,PRTY,SRC_SME_ID,DST_SME_ID,
    TXT_LENGTH,MSG_REF,SEQ_NUM,
    BODY_LEN,BODY
from sms_msg
;

DROP TABLE SMS_ATCH_BAK;
CREATE TABLE SMS_ATCH_BAK
(
   ID       RAW(8)   NOT NULL,
   BODY     BLOB     NOT NULL
) TABLESPACE SMSC_DATA;

insert into sms_atch_bak
select id, body
from sms_atch;

DROP TABLE SMS_IDS_bak;
CREATE TABLE SMS_IDS_bak
(  
   ID        RAW(8) NOT NULL 
)

insert into sms_ids_bak
select id from sms_ids;

DROP TABLE SMS_ARC_BAK;
CREATE TABLE SMS_ARC_BAK
(  
   ID             RAW(8)   NOT NULL,
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

insert into sms_arc_bak
select
  ID,ST,SUBMIT_TIME,VALID_TIME,ATTEMPTS,
  LAST_RESULT,LAST_TRY_TIME,NEXT_TRY_TIME,
  OA,DA,DDA,MR,SVC_TYPE,DR,BR,
  SRC_MSC,SRC_IMSI,SRC_SME_N,DST_MSC,DST_IMSI,DST_SME_N,
  ROUTE_ID,SVC_ID,PRTY,SRC_SME_ID,DST_SME_ID,
  TXT_LENGTH,BODY_LEN,BODY
from sms_arc;

DROP TABLE SMS_BILL_BAK;
CREATE TABLE SMS_BILL_BAK
(
   ID             NUMBER         NOT NULL,
   MSG_ID         RAW(8)         NOT NULL,
   CALL_DIRECTION VARCHAR2(5)    NOT NULL,
   RECORD_TYPE    NUMBER         NOT NULL,
   SUBMIT         DATE           NOT NULL,
   FINALIZED      DATE           NULL,
   STATUS         NUMBER         NOT NULL,
   PAYER_ADDR     VARCHAR2(21)   NOT NULL,
   PAYER_TON      NUMBER         NOT NULL,
   PAYER_NPI      NUMBER         NOT NULL,
   PAYER_IMSI     VARCHAR2(21)   NULL,
   PAYER_MSC      VARCHAR2(21)   NULL,
   OTHER_ADDR     VARCHAR2(21)   NULL,
   OTHER_TON      NUMBER         NULL,
   OTHER_NPI      NUMBER         NULL,
   ROUTE_ID       VARCHAR2(32)   NULL,
   SERVICE_CODE   NUMBER         NULL,
   TXT_LENGTH     NUMBER         NULL,
   UPLOAD_ID      NUMBER         NULL
) TABLESPACE SMSC_DATA;

insert into sms_bill_bak
select
  ID,MSG_ID,CALL_DIRECTION,RECORD_TYPE,SUBMIT,FINALIZED,STATUS,
  PAYER_ADDR,PAYER_TON,PAYER_NPI,PAYER_IMSI,PAYER_MSC,OTHER_ADDR,    
  OTHER_TON,OTHER_NPI,ROUTE_ID,SERVICE_CODE,TXT_LENGTH,UPLOAD_ID
from sms_bill;
