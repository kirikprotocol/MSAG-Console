alter table sms_profile add
(
  REPORTINFO_TMP CHAR(3),
  CODESET_TMP    CHAR(3),
  HIDDEN_TMP     CHAR,
  UDHCONCAT      CHAR
);

update sms_profile set 
  REPORTINFO_TMP=TO_CHAR(REPORTINFO),
  REPORTINFO=NULL,
  CODESET_TMP=TO_CHAR(CODESET),
  CODESET=NULL,
  HIDDEN_TMP=TRANSLATE(TO_CHAR(HIDDEN),'01','NY'),
  HIDDEN=NULL;

alter table sms_profile modify
(
  REPORTINFO CHAR(3),
  CODESET    CHAR(3),
  HIDDEN     CHAR,
  DIVERT_ACT CHAR(5)
);
update sms_profile set 
  REPORTINFO=REPORTINFO_TMP,
  CODESET=CODESET_TMP,
  HIDDEN=HIDDEN_TMP;
alter table sms_profile drop column REPORTINFO_TMP;
alter table sms_profile drop column CODESET_TMP;
alter table sms_profile drop column HIDDEN_TMP;

