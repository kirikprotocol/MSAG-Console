
-- ******************* Distribution lists tables ******************* --

--    Contains distribution lists   --
DROP TABLE DL_SET;
CREATE TABLE DL_SET
(
   LIST        VARCHAR2       NOT NULL,
   MAX_EL      NUMBER(22)     NOT NULL,
   OWNER       VARCHAR2(30)   NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX DL_SET_IDX ON DL_SET (LIST) TABLESPACE SMSC_IDX;
ALTER TABLE DL_SET ADD CONSTRAINT DL_SET_PK PRIMARY KEY (LIST);

--    Contains distribution list members  --
DROP TABLE DL_MEMBERS;
CREATE TABLE DL_MEMBERS
(
   LIST        VARCHAR2       NOT NULL,
   ADDRESS     VARCHAR2(30)   NOT NULL
) TABLESPACE SMSC_DATA;

--    Contains distribution list submitters  --
DROP TABLE DL_SUBMITTERS;
CREATE TABLE DL_SUBMITTERS
(
   ADDRESS     VARCHAR2(30)   NOT NULL,
   LIST        VARCHAR2       NOT NULL
) TABLESPACE SMSC_DATA;

CREATE INDEX DL_SUBMITTERS_IDX ON DL_SUBMITTERS (ADDRESS, LIST) TABLESPACE SMSC_IDX;
ALTER TABLE DL_SUBMITTERS ADD CONSTRAINT DL_SUBMITTERS_PK PRIMARY KEY (ADDRESS, LIST);

--    Contains users principals   --
DROP TABLE DL_PRINCIPALS;
CREATE TABLE DL_PRINCIPALS
(
   ADDRESS     VARCHAR2(30)   NOT NULL,
   MAX_LST     NUMBER(22)     NOT NULL,
   MAX_EL      NUMBER(22)     NOT NULL,
) TABLESPACE SMSC_DATA;
