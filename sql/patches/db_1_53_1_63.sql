create or replace function to_dec
( p_str in varchar2, 
  p_from_base in number default 16 ) return number
is
		 l_num   number default 0;
		 l_hex   varchar2(16) default '0123456789ABCDEF';
begin
		 for i in 1 .. length(p_str) loop
		 		 l_num := l_num * p_from_base + instr(l_hex,upper(substr(p_str,i,1)))-1;
		 end loop;
		 return l_num;
end to_dec;
/

create table sms_msg_bak as select * from sms_msg;

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
   MSG_REF        NUMBER(3)    NULL,
   SEQ_NUM        NUMBER(3)    NOT NULL,
   BODY_LEN       NUMBER(10)   NOT NULL,
   BODY           RAW(1500)    NULL
) TABLESPACE SMSC_DATA INITRANS 40;

insert into sms_msg
select 
    to_dec(rawtohex(ID)),ST,SUBMIT_TIME,VALID_TIME,ATTEMPTS,
    LAST_RESULT,LAST_TRY_TIME,NEXT_TRY_TIME,
    OA,DA,DDA,MR,SVC_TYPE,ARC,DR,BR,
    SRC_MSC,SRC_IMSI,SRC_SME_N,
    DST_MSC,DST_IMSI,DST_SME_N,
    ROUTE_ID,SVC_ID,PRTY,SRC_SME_ID,DST_SME_ID,
    TXT_LENGTH,MSG_REF,SEQ_NUM,
    BODY_LEN,BODY
from sms_msg_bak
;

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

create table SMS_ATCH_BAK as select * from SMS_ATCH;

DROP TABLE SMS_ATCH;
CREATE TABLE SMS_ATCH
(
   ID       NUMBER(22)   NOT NULL
            CONSTRAINT SMS_ATCH_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX  INITRANS 40,
   BODY     BLOB     NOT NULL
) TABLESPACE SMSC_DATA  INITRANS 40;

insert into sms_atch
select to_dec(rawtohex(ID)), body
from sms_atch_bak;


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

DROP SEQUENCE SMS_BILL_SEQ;
CREATE SEQUENCE SMS_BILL_SEQ INCREMENT BY 1 START WITH 1 CACHE 200;

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
   A_bodyLen IN NUMBER, A_body IN RAW, A_arc IN NUMBER, A_bill IN NUMBER)
IS
   billId            NUMBER;
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
   
   IF A_bill != 0 THEN
   	SELECT SMS_BILL_SEQ.NEXTVAL INTO billId FROM DUAL;
   	INSERT INTO SMS_BILL 
            (ID, MSG_ID, CALL_DIRECTION, RECORD_TYPE, SUBMIT, FINALIZED, STATUS,
             PAYER_ADDR, PAYER_TON, PAYER_NPI, PAYER_IMSI, PAYER_MSC,
             OTHER_ADDR, OTHER_TON, OTHER_NPI,
             ROUTE_ID, SERVICE_CODE, TXT_LENGTH) 
	VALUES 
            (billId, A_id, 'O', 10, A_submitTime, A_lastTryTime, A_st, 
      	     A_oaVal, A_oaTon, A_oaNpi, A_srcImsi, A_srcMsc,
             A_daVal, A_daTon, A_daNpi, 
             A_routeId, A_svcId, A_txtLength);
   
        SELECT SMS_BILL_SEQ.NEXTVAL INTO billId FROM DUAL;
        INSERT INTO SMS_BILL 
            (ID, MSG_ID, CALL_DIRECTION, RECORD_TYPE, SUBMIT, FINALIZED, STATUS,
             PAYER_ADDR, PAYER_TON, PAYER_NPI, PAYER_IMSI, PAYER_MSC,
             OTHER_ADDR, OTHER_TON, OTHER_NPI, 
             ROUTE_ID, SERVICE_CODE, TXT_LENGTH) 
        VALUES 
            (billId, A_id, 'I', 20, A_submitTime, A_lastTryTime, A_st, 
             A_daVal, A_daTon, A_daNpi, A_dstImsi, A_dstMsc,
             A_oaVal, A_oaTon, A_oaNpi, 
             A_routeId, A_svcId, A_txtLength);
   END IF;
   
   DELETE FROM SMS_MSG WHERE ID=A_id;

END DO_FINALIZE_SMS;
/

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


-- ********************** Billing table for SMSC ********************** --
DROP PROCEDURE CREATE_BILLING_RECORD;

DROP TABLE SMS_BILL;
CREATE TABLE SMS_BILL
(
   ID             NUMBER         NOT NULL    -- generated via sequence
   		  CONSTRAINT SMS_BILL_PK PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX INITRANS 40,
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
) TABLESPACE SMSC_DATA INITRANS 40;

CREATE INDEX SMS_BILL_UPLOAD_ID_IDX ON SMS_BILL (UPLOAD_ID) 
TABLESPACE SMSC_IDX INITRANS 40;
