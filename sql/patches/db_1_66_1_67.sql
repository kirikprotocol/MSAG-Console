CREATE OR REPLACE PROCEDURE DO_FINALIZE_SMS 
  (A_id IN NUMBER, A_st IN NUMBER, A_submitTime IN DATE, A_validTime IN DATE,
   A_attempts IN NUMBER, A_lastResult IN NUMBER, 
   A_lastTryTime IN DATE, A_nextTryTime IN DATE,
   A_oa IN VARCHAR2, A_oaVal IN VARCHAR2, A_oaTon IN NUMBER, A_oaNpi IN NUMBER,
   A_da IN VARCHAR2, A_daVal IN VARCHAR2, A_daTon IN NUMBER, A_daNpi IN NUMBER,
   A_dda IN VARCHAR2,
   A_mr IN NUMBER, A_svcType IN VARCHAR2, A_dr IN NUMBER, A_br IN NUMBER,
   A_srcMsc IN VARCHAR2, A_srcImsi IN VARCHAR2, A_srcSmeN IN NUMBER,
   A_dstMsc IN VARCHAR2, A_dstImsi IN VARCHAR2, A_dstSmeN IN NUMBER,
   A_routeId IN VARCHAR2, A_svcId IN NUMBER, A_prty IN NUMBER, 
   A_srcSmeId IN VARCHAR2, A_dstSmeId VARCHAR2, A_txtLength IN NUMBER,
   A_bodyLen IN NUMBER, A_body IN RAW, A_arc IN NUMBER, A_bill IN NUMBER, 
   A_delete IN NUMBER, A_diverted IN NUMBER)
IS
   billId            NUMBER;
BEGIN
   
   IF A_arc != 0 THEN
    INSERT INTO SMS_ARC
	(ID, ST, SUBMIT_TIME, VALID_TIME, ATTEMPTS, LAST_RESULT,
	 LAST_TRY_TIME, NEXT_TRY_TIME, OA, DA, DDA, MR, SVC_TYPE, DR, BR, 
	 SRC_MSC, SRC_IMSI, SRC_SME_N, DST_MSC, DST_IMSI, DST_SME_N,
	 ROUTE_ID, SVC_ID, PRTY, SRC_SME_ID, DST_SME_ID, 
	 TXT_LENGTH, BODY_LEN, BODY) 
    VALUES
	(A_id, A_st, A_submitTime, A_validTime, A_attempts, A_lastResult,
	 A_lastTryTime, A_nextTryTime, A_oa, A_da, A_dda, A_mr, A_svcType, A_dr, A_br,
	 A_srcMsc, A_srcImsi, A_srcSmeN, A_dstMsc, A_dstImsi, A_dstSmeN,
	 A_routeId, A_svcId, A_prty, A_srcSmeId, A_dstSmeId, 
	 A_txtLength, A_bodyLen, A_body);
   END IF;
   
   IF A_bill != 0 THEN
   	SELECT SMS_BILL_SEQ.NEXTVAL INTO billId FROM DUAL;
   	INSERT INTO SMS_BILL 
            (ID, MSG_ID, CALL_DIRECTION, RECORD_TYPE, SUBMIT, FINALIZED, STATUS,
             PAYER_ADDR, PAYER_TON, PAYER_NPI, PAYER_IMSI, PAYER_MSC,
             OTHER_ADDR, OTHER_TON, OTHER_NPI,
             ROUTE_ID, SERVICE_CODE, TXT_LENGTH) 
	VALUES 
            (billId, A_id, 'O', 10, A_submitTime, A_lastTryTime, A_st, 
      	     A_oaVal, A_oaTon, A_oaNpi, A_srcImsi, A_srcMsc,
             A_daVal, A_daTon, A_daNpi, 
             A_routeId, A_svcId, A_txtLength);
   
        SELECT SMS_BILL_SEQ.NEXTVAL INTO billId FROM DUAL;
        INSERT INTO SMS_BILL 
            (ID, MSG_ID, CALL_DIRECTION, RECORD_TYPE, SUBMIT, FINALIZED, STATUS,
             PAYER_ADDR, PAYER_TON, PAYER_NPI, PAYER_IMSI, PAYER_MSC,
             OTHER_ADDR, OTHER_TON, OTHER_NPI, 
             ROUTE_ID, SERVICE_CODE, TXT_LENGTH) 
        VALUES 
            (billId, A_id, 'I', 20, A_submitTime, A_lastTryTime, A_st, 
             A_daVal, A_daTon, A_daNpi, A_dstImsi, A_dstMsc,
             A_oaVal, A_oaTon, A_oaNpi, 
             A_routeId, A_svcId, A_txtLength);
        IF A_diverted != 0 THEN
            SELECT SMS_BILL_SEQ.NEXTVAL INTO billId FROM DUAL;
            INSERT INTO SMS_BILL 
                (ID, MSG_ID, CALL_DIRECTION, RECORD_TYPE, SUBMIT, FINALIZED, STATUS,
                 PAYER_ADDR, PAYER_TON, PAYER_NPI, PAYER_IMSI, PAYER_MSC,
                 OTHER_ADDR, OTHER_TON, OTHER_NPI,
                 ROUTE_ID, SERVICE_CODE, TXT_LENGTH) 
            VALUES 
                (billId, A_id, 'O', 30, A_submitTime, A_lastTryTime, A_st, 
      	         A_oaVal, A_oaTon, A_oaNpi, A_srcImsi, A_srcMsc,
                 A_daVal, A_daTon, A_daNpi, 
                 A_routeId, A_svcId, A_txtLength);
        END IF;
   END IF;
   
   IF A_delete != 0 THEN
       DELETE FROM SMS_MSG WHERE ID=A_id;
   END IF;

END DO_FINALIZE_SMS;
/
