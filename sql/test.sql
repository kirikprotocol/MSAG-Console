CREATE OR REPLACE PROCEDURE insert_record(ID_ INT, ST_ NUMBER, MR_ NUMBER, RM_ NUMBER, OA_LEN_ NUMBER, OA_TON_ NUMBER, OA_NPI_ NUMBER, OA_VAL_ VARCHAR2, DA_LEN_ NUMBER, DA_TON_ NUMBER, DA_NPI_ NUMBER, DA_VAL_ VARCHAR2, VALID_TIME_ DATE, WAIT_TIME_ DATE, SUBMIT_TIME_ DATE, DELIVERY_TIME_ DATE, SRR_ CHAR, RD_ CHAR, PRI_ NUMBER, PID_ NUMBER, FCS_ NUMBER, DCS_ NUMBER, UDHI_ CHAR, UDL_ NUMBER, UD_ RAW) IS
BEGIN
   insert into sms_msg (ID, ST, MR, RM, OA_LEN, OA_TON, OA_NPI, OA_VAL, DA_LEN, DA_TON, DA_NPI, DA_VAL, VALID_TIME, WAIT_TIME,	SUBMIT_TIME, DELIVERY_TIME, SRR, RD, PRI, PID, FCS, DCS, UDHI, UDL, UD)
   values (ID_, ST_, MR_, RM_, OA_LEN_, OA_TON_, OA_NPI_, OA_VAL_, DA_LEN_, DA_TON_, DA_NPI_, DA_VAL_, VALID_TIME_, WAIT_TIME_, SUBMIT_TIME_, DELIVERY_TIME_, SRR_, RD_, PRI_, PID_, FCS_, DCS_, UDHI_, UDL_, UD_);
   commit;
END;
/

CREATE OR REPLACE PROCEDURE TEST(num NUMBER) IS
   last_id NUMBER;
   max_id NUMBER;
   i NUMBER;
   t1 DATE;
   t2 DATE;
   dt FLOAT;
   n1 NUMBER;
   n2 NUMBER;
   dn NUMBER;
   rate FLOAT;
BEGIN
   select nvl(max(id),0) into last_id from sms_msg;
   i := last_id + 1;
   max_id := last_id + num;
   select count(*) into n1 from sms_msg; 
   select sysdate into t1 from dual;
   while i <= max_id loop
      --sms_msg:    ID, ST, MR, RM, OA_LEN, OA_TON, OA_NPI,                  OA_VAL, DA_LEN, DA_TON, DA_NPI,                  DA_VAL, VALID_TIME, WAIT_TIME, SUBMIT_TIME, DELIVERY_TIME, SRR,  RD, PRI, PID, FCS, DCS, UDHI, UDL, UD
      insert_record( i,  0, 69, 71,     21,     20,     30, '123456789012345678901',     21,     20,     30, '123456789012345678901',    sysdate,   sysdate,     sysdate,       sysdate, 'Y', 'Y', 100, 150,  55,  20,  'N', 200, hextoraw('313839453435338414C4947495946475559464359544448474B4D54F4A494849464448544B485495554F5949465554464C494A4F49555494F594F4947484B55484B4C554748594A46555446545548544459525345524145575847564E42494A54F4A4F549484959475954445246434E424859465954445254525344545344554755544446595244544452434B42484B4C4A494F4849475546544452534454455346484C4A4F4954A554F4946544454444A48564B48554F5949484C49484759464A54464A4C4C4C4'));
      i := i + 1;
   end loop;
   select sysdate into t2 from dual;
   select count(*) into n2 from sms_msg; 
   dt := (t2 - t1) * 3600 * 24;
   dn := n2 - n1;
   if dt > 0 then
      rate := num / dt;
   else
      rate := 0;
   end if;
   dbms_output.put_line('dt = ' || dt || ', dn = ' || dn || ', rate = ' || rate);
END;
/
