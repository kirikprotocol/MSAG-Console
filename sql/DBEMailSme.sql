DROP TABLE EMLSME_PROFILES;
CREATE TABLE EMLSME_PROFILES(
  ADDRESS VARCHAR(21),
  USERNAME VARCHAR(64),
  FORWARD VARCHAR(64),
  DAILY_LIMIT NUMBER(9)
)TABLESPACE SMSC_DATA;


CREATE UNIQUE INDEX EMLSME_IDX_PHONE ON EMLSME_PROFILES(ADDRESS)
TABLESPACE SMSC_IDX;

CREATE UNIQUE INDEX EMLSME_IDX_EMAIL ON EMLSME_PROFILES(USERNAME)
TABLESPACE SMSC_IDX;

DROP TABLE EMLSME_HISTORY;
CREATE TABLE EMLSME_HISTORY(
  ADDRESS VARCHAR(21) NOT NULL,
  MSG_DATE DATE
);

CREATE INDEX EMLSME_IDX_HISTORY_DATE ON EMLSME_HISTORY(MSG_DATE)
TABLESPACE SMSC_IDX;

CREATE INDEX EMLSME_IDX_HISTORY_ADDR ON EMLSME_HISTORY(ADDRESS)
TABLESPACE SMSC_IDX;

