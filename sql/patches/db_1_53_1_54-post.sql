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

insert into sms_atch
select to_dec(rawtohex(ID)), body
from sms_atch_bak;

insert into sms_ids
select to_dec(rawtohex(ID)) from sms_ids_bak;

insert into sms_arc
select
  to_dec(rawtohex(ID)),ST,SUBMIT_TIME,VALID_TIME,ATTEMPTS,
  LAST_RESULT,LAST_TRY_TIME,NEXT_TRY_TIME,
  OA,DA,DDA,MR,SVC_TYPE,DR,BR,
  SRC_MSC,SRC_IMSI,SRC_SME_N,DST_MSC,DST_IMSI,DST_SME_N,
  ROUTE_ID,SVC_ID,PRTY,SRC_SME_ID,DST_SME_ID,
  TXT_LENGTH,BODY_LEN,BODY
from sms_arc_bak;

insert into sms_bill
select
  ID,to_dec(rawtohex(MSG_ID)),CALL_DIRECTION,RECORD_TYPE,SUBMIT,FINALIZED,STATUS,
  PAYER_ADDR,PAYER_TON,PAYER_NPI,PAYER_IMSI,PAYER_MSC,OTHER_ADDR,    
  OTHER_TON,OTHER_NPI,ROUTE_ID,SERVICE_CODE,TXT_LENGTH,UPLOAD_ID
from sms_bill_bak;

drop function to_dec;

