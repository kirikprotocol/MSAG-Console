
DROP SEQUENCE SMS_MSG_ID_SEQUENCE;
CREATE SEQUENCE SMS_MSG_ID_SEQUENCE 
	INCREMENT BY 1 
	START WITH 1 
	NOMAXVALUE 
	ORDER;                                                                         

DROP TABLE SMS_ID_LOCK;
CREATE TABLE SMS_ID_LOCK 
(
	TGT	VARCHAR2(64)	PRIMARY KEY USING INDEX TABLESPACE SMSC_IDX,
	ID	INT		NOT NULL
) TABLESPACE SMSC_DATA INITRANS 200 NOLOGGING;

INSERT INTO SMS_ID_LOCK VALUES ('SMS_MSG_TABLE', 0);*/

CREATE OR REPLACE PROCEDURE INSERT_NEW_MSG 
   (id IN OUT NUMBER, st NUMBER, mr NUMBER, rm NUMBER,
    oa_len NUMBER, oa_ton NUMBER, oa_npi NUMBER, oa_val VARCHAR2,
    da_len NUMBER, da_ton NUMBER, da_npi NUMBER, da_val VARCHAR2,
    valid_time DATE, wait_time DATE, submit_time DATE, delivery_time DATE,
    srr CHAR, rd CHAR, pri NUMBER, pid NUMBER, fcs NUMBER,
    dcs NUMBER, udhi CHAR, udl NUMBER, ud RAW)
    IS 
BEGIN
	SELECT SMS_MSG_ID_SEQUENCE.NEXTVAL INTO id FROM DUAL;
	INSERT INTO SMS_MSG VALUES 
        (id, st, mr, rm,
         oa_len, oa_ton, oa_npi, oa_val, da_len, da_ton, da_npi, da_val,
         valid_time, wait_time, submit_time, delivery_time,
         srr, rd, pri, pid, fcs, dcs, udhi, udl, ud);
END INSERT_NEW_MSG;

/*CREATE INDEX ID_IDX ON SMS_MSG (ID ASC) 
NOSORT NOCOMPRESS NOLOGGING PARALLEL
TABLESPACE SMSC1;*/

/*CREATE OR REPLACE FUNCTION insert_new_msg 
       (id NUMBER, st NUMBER, mr NUMBER, rm NUMBER,
	oa_len NUMBER, oa_ton NUMBER, oa_npi NUMBER, oa_val VARCHAR2,
	da_len NUMBER, da_ton NUMBER, da_npi NUMBER, da_val VARCHAR2,
	valid_time DATE, wait_time DATE, submit_time DATE, delivery_time DATE,
	srr CHAR, rd CHAR, pri NUMBER, pid NUMBER,
	fcs NUMBER, dcs NUMBER, udhi CHAR, udl NUMBER, ud RAW)
RETURN NUMBER IS 
msg_id NUMBER;
BEGIN
	SELECT SMS_MSG_ID_SEQUENCE.NEXTVAL INTO msg_id FROM DUAL;
	INSERT INTO SMS_MSG VALUES (msg_id, st, mr, rm, 
		oa_len, oa_ton, oa_npi, oa_val, da_len, da_ton, da_npi, da_val,
		valid_time, wait_time, submit_time, delivery_time,
       		srr, rd, pri, pid, fcs, dcs, udhi, udl, ud);
	RETURN (msg_id);
END insert_new_msg;*/

 