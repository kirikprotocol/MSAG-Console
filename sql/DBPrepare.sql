DROP TABLE SMS_MSG;
CREATE TABLE SMS_MSG
(	
	ID 		RAW(8) 		NOT NULL,
	ST 		NUMBER(3)	NOT NULL,
        MR 		NUMBER(3)	NOT NULL,
	OA_TON 		NUMBER(3)	NOT NULL,
	OA_NPI 		NUMBER(3)	NOT NULL,
	OA_VAL		VARCHAR(21)	NOT NULL,	
	SRC_MSC		VARCHAR(21)	NOT NULL,
	SRC_IMSI	VARCHAR(21)	NOT NULL,
	SRC_SME_N	NUMBER(22)	NOT NULL,	
	DA_TON 		NUMBER(3)	NOT NULL,
	DA_NPI 		NUMBER(3)	NOT NULL,
	DA_VAL 		VARCHAR(21)	NOT NULL,	
	DST_MSC		VARCHAR(21)	NULL,
	DST_IMSI	VARCHAR(21)	NULL,
	DST_SME_N	NUMBER(22)	NULL,
	WAIT_TIME	DATE		NULL,
	VALID_TIME	DATE		NOT NULL,
	SUBMIT_TIME	DATE		NOT NULL,	
	ATTEMPTS	NUMBER(3)	NOT NULL,
	LAST_RESULT	NUMBER(3)	NOT NULL,
	LAST_TRY_TIME	DATE		NULL,
	NEXT_TRY_TIME	DATE		NULL,	
	SVC_TYPE	VARCHAR(6)	NULL,
	DR		NUMBER(3)	NOT NULL,
	ARC		CHAR(1)		NOT NULL,
	PRI		NUMBER(3)	NOT NULL,
	PID		NUMBER(3)	NOT NULL,
	UDHI		CHAR(1)		NOT NULL,
	DCS		NUMBER(3)	NOT NULL,
	UDL		NUMBER(3)	NOT NULL,
	UD		RAW(200)	NOT NULL
) TABLESPACE SMSC_DATA;

CREATE UNIQUE INDEX SMS_MSG_IDX ON SMS_MSG (ID)
TABLESPACE SMSC_IDX NOSORT;

ALTER TABLE SMS_MSG ADD CONSTRAINT SMS_MSG_PK PRIMARY KEY (ID);

CREATE INDEX SMS_MSG_OA_IDX ON SMS_MSG (OA_TON, OA_NPI, OA_VAL)
TABLESPACE SMSC_IDX;

CREATE INDEX SMS_MSG_DA_IDX ON SMS_MSG (DA_TON, DA_NPI, DA_VAL)
TABLESPACE SMSC_IDX;

DROP TABLE SMS_ARC;
CREATE TABLE SMS_ARC
(	
	ID 		RAW(8) 		NOT NULL,
	ST 		NUMBER(3)	NOT NULL,
        MR 		NUMBER(3)	NOT NULL,
	OA_TON 		NUMBER(3)	NOT NULL,
	OA_NPI 		NUMBER(3)	NOT NULL,
	OA_VAL		VARCHAR(21)	NOT NULL,	
	SRC_MSC		VARCHAR(21)	NOT NULL,
	SRC_IMSI	VARCHAR(21)	NOT NULL,
	SRC_SME_N	NUMBER(22)	NOT NULL,	
	DA_TON 		NUMBER(3)	NOT NULL,
	DA_NPI 		NUMBER(3)	NOT NULL,
	DA_VAL 		VARCHAR(21)	NOT NULL,	
	DST_MSC		VARCHAR(21)	NULL,
	DST_IMSI	VARCHAR(21)	NULL,
	DST_SME_N	NUMBER(22)	NULL,
	WAIT_TIME	DATE		NULL,
	VALID_TIME	DATE		NOT NULL,
	SUBMIT_TIME	DATE		NOT NULL,	
	ATTEMPTS	NUMBER(3)	NOT NULL,
	LAST_RESULT	NUMBER(3)	NOT NULL,
	LAST_TRY_TIME	DATE		NULL,
	DR		NUMBER(3)	NOT NULL,
	PRI		NUMBER(3)	NOT NULL,
	PID		NUMBER(3)	NOT NULL,
	UDHI		CHAR(1)		NOT NULL,
	DCS		NUMBER(3)	NOT NULL,
	UDL		NUMBER(3)	NOT NULL,
	UD		RAW(200)	NOT NULL
) TABLESPACE SMSC_DATA;

CREATE UNIQUE INDEX SMS_ARC_IDX ON SMS_ARC (ID)
TABLESPACE SMSC_IDX;

ALTER TABLE SMS_ARC ADD CONSTRAINT SMS_ARC_PK PRIMARY KEY (ID);

DROP TABLE SMS_BR;
CREATE TABLE SMS_BR
(	
	N		NUMBER(22)	NULL,
	TD		DATE		NULL,
	FD		DATE		NULL,
	SMS_UPLOAD_N	NUMBER(22)	NULL,
	CI		NUMBER(22)	NULL,
        SRC_ADDR	VARCHAR2(21)	NOT NULL,
	SRC_TON		VARCHAR2(21)	NOT NULL,
	SRC_NPI		VARCHAR2(21)	NOT NULL,
	SRC_MSC		VARCHAR2(21)	NULL,
	SRC_IMSI	VARCHAR2(21)	NULL,
        SRC_SME_N       NUMBER(22)	NULL,
        DST_ADDR	VARCHAR2(21)	NOT NULL,
	DST_TON		VARCHAR2(21)	NOT NULL,
	DST_NPI		VARCHAR2(21)	NOT NULL,
	DST_MSC		VARCHAR2(21)	NULL,
	DST_IMSI	VARCHAR2(21)	NULL,
	DST_SME_N       NUMBER(22)	NULL,
	SUBMIT_FD	DATE		NOT NULL,
	STATUS          NUMBER(22)	NOT NULL,
	ATTEMPT		NUMBER(22)	NOT NULL,
	LAST_RESULT	NUMBER(22)	NOT NULL,
        PRTY		NUMBER(22)	NOT NULL,
	RP		NUMBER(22)	NOT NULL,
        TXT_LENGTH	NUMBER(22)	NOT NULL,
	DATA		VARCHAR2(2000)	NULL
) TABLESPACE SMSC_DATA;

DROP TABLE SMS_IDS;
CREATE TABLE SMS_IDS
(	
	ID 		RAW(8) 		NOT NULL
) TABLESPACE SMSC_DATA;

CREATE UNIQUE INDEX SMS_IDS_IDX ON SMS_IDS (ID)
TABLESPACE SMSC_IDX;
