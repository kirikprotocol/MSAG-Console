alter table sms_profile add (
  TRANSLIT CHAR
);
update sms_profile set translit='Y';
quit;
