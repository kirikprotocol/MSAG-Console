
-- ********************* Operative tables for MCISme ********************* --

DROP SEQUENCE MCISME_MSG_SEQ;
CREATE SEQUENCE MCISME_MSG_SEQ INCREMENT BY 1000
START WITH 0 MINVALUE 0 MAXVALUE 10000000000000000000000 CACHE 100;

DROP TABLE MCISME_MSG_SET;
CREATE TABLE MCISME_MSG_SET
(
    ID		NUMBER(22)	NOT NULL
	        CONSTRAINT MCISME_MSG_SET_PK PRIMARY KEY,
    STATE	NUMBER(3)	NOT NULL,
    ABONENT	VARCHAR2(30)	NOT NULL,
    SMSC_ID	VARCHAR2(64)	NULL
);

CREATE INDEX MCISME_MSG_SET_STATE_IDX   ON MCISME_MSG_SET (STATE);
CREATE INDEX MCISME_MSG_SET_ABONENT_IDX ON MCISME_MSG_SET (ABONENT);
CREATE INDEX MCISME_MSG_SET_SMSC_ID_IDX ON MCISME_MSG_SET (SMSC_ID);

DROP TABLE MCISME_EVT_SET;
CREATE TABLE MCISME_EVT_SET
(
    ID		NUMBER(22)	NOT NULL
	        CONSTRAINT MCISME_EVT_SET_PK PRIMARY KEY,
    ABONENT	VARCHAR2(30)	NOT NULL,
    DT		DATE		NOT NULL,
    CALLER	VARCHAR2(30)	NULL, -- NULL if caller is undefined
    MSG_ID	NUMBER(22)	NULL  -- NULL if not assigned to message
);

CREATE INDEX MCISME_EVT_SET_ABONENT_IDX ON MCISME_EVT_SET (ABONENT);
CREATE INDEX MCISME_EVT_SET_MSG_ID_IDX  ON MCISME_EVT_SET (MSG_ID);

DROP TABLE MCISME_CUR_MSG;
CREATE TABLE MCISME_CUR_MSG
(
    ABONENT	VARCHAR2(30)	NOT NULL
		CONSTRAINT MCISME_CUR_MSG_ABONENT_PK PRIMARY KEY,
    ID		NUMBER(22)	NOT NULL
);

DROP TABLE MCISME_ABONENTS;
CREATE TABLE MCISME_ABONENTS
(
    ABONENT	VARCHAR2(30)	NOT NULL
		CONSTRAINT MCISME_ABONENTS_ABONENT_PK PRIMARY KEY,
    SERVICE	NUMBER(3)	NULL
);

DROP TABLE MCISME_STAT;
CREATE TABLE MCISME_STAT
(
    PERIOD	NUMBER(22)	NOT NULL,
    GENERATED	NUMBER(22)	NOT NULL,
    DELIVERED	NUMBER(22)	NOT NULL,
    RETRIED	NUMBER(22)	NOT NULL,
    FAILED	NUMBER(22)	NOT NULL
);

CREATE INDEX MCISME_PERIOD_IDX ON MCISME_STAT (PERIOD);

COMMIT;
QUIT;