-- ********************* Testing tables for SmsExport without field ARC********************* --
-- Таблица для SMS экспорта ( в дальнейшем надо добавить поле ARC )
CREATE TABLE OPER_SMS
(
   ID             NUMBER(22)   NOT NULL
                  CONSTRAINT SMS_MSG_PK1 PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX INITRANS 40,
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
   MSG_REF        NUMBER(5)    NULL,
   SEQ_NUM        NUMBER(3)    NOT NULL,
   BODY_LEN       NUMBER(10)   NOT NULL
   );
 /
--создаётся один раз

CREATE OR REPLACE TYPE sms AS OBJECT
(
   ID             NUMBER(22)   ,
   ST             NUMBER(3)    ,
   SUBMIT_TIME    DATE         ,
   VALID_TIME     DATE         ,
   ATTEMPTS       NUMBER(22)   ,
   LAST_RESULT    NUMBER(22)   ,
   LAST_TRY_TIME  DATE         ,
   NEXT_TRY_TIME  DATE         ,
   OA             VARCHAR2(30) ,
   DA             VARCHAR2(30) ,
   DDA            VARCHAR2(30) ,
   MR             NUMBER(5)    ,
   SVC_TYPE       VARCHAR2(6)  ,
   DR             NUMBER(3)    ,
   BR             NUMBER(3)    ,
   SRC_MSC        VARCHAR2(21) ,
   SRC_IMSI       VARCHAR2(21) ,
   SRC_SME_N      NUMBER(22)   ,
   DST_MSC        VARCHAR2(21) ,
   DST_IMSI       VARCHAR2(21) ,
   DST_SME_N      NUMBER(22)   ,
   ROUTE_ID       VARCHAR2(32) ,
   SVC_ID         NUMBER(22)   ,
   PRTY           NUMBER(22)   ,
   SRC_SME_ID     VARCHAR2(15) ,
   DST_SME_ID     VARCHAR2(15) ,
   MSG_REF        NUMBER(5)    ,
   SEQ_NUM        NUMBER(3)    ,
   BODY_LEN       NUMBER(10)
);
/

CREATE OR REPLACE TYPE arraylist AS ARRAY(1000) OF sms;
/

-- создаётся из Java с текущим именем таблицы + создать один раз вначале для определения процедуры insert_rec();

create or replace procedure insert_rec( msg sms) is
begin
         insert into oper_sms (id,st,submit_time,valid_time,attempts,last_result,
		last_try_time,next_try_time,oa,da,dda,mr,svc_type,dr,br,src_msc,
		src_imsi,src_sme_n,dst_msc,dst_imsi,dst_sme_n,route_id,svc_id,prty,
                src_sme_id,dst_sme_id,msg_ref,seq_num,body_len)
	values (msg.id,msg.st,msg.submit_time,msg.valid_time,msg.attempts,
		msg.last_result,msg.last_try_time,msg.next_try_time,msg.oa,msg.da,
		msg.dda,msg.mr,msg.svc_type,msg.dr,msg.br,msg.src_msc,
		msg.src_imsi,msg.src_sme_n,msg.dst_msc,msg.dst_imsi,msg.dst_sme_n,
		msg.route_id,msg.svc_id,msg.prty,msg.src_sme_id,msg.dst_sme_id,
                msg.msg_ref,msg.seq_num,msg.body_len);
end;
/

--создаётся один раз
create or replace procedure multinsert_sms(maxRow  integer,msg  sms ) is
        i number(6);
	msg sms;
begin

	while i <= maxRow loop
                msg:=msgs(i);
		insert_rec(msg);
		i := i + 1;
	end loop;
	commit;
end;
/

-- собственно вызов процедуры
-- CALL multinsert_sms(maxRow,msgs arraylist);

